param([string]$Out)
# Отрисовка диаграмм и макетов в PNG по модели parking-system.sysml и ui-mockups.sysml.
# Масштаб: 1950 точек = 165 мм печати; шрифт 58 точек = 14 пт, строчные буквы не ниже 2,5 мм (СТП 01-2024, п. 2.5.8).
Add-Type -AssemblyName System.Drawing
$FS = 58
$black = [Drawing.Color]::Black
$fill = [Drawing.Color]::FromArgb(238,238,238)
$dark = [Drawing.Color]::FromArgb(205,205,205)

function New-Canvas($w,$h){
  $b = New-Object Drawing.Bitmap $w,$h
  $b.SetResolution(300,300)
  $g = [Drawing.Graphics]::FromImage($b)
  $g.SmoothingMode='AntiAlias'; $g.TextRenderingHint='AntiAliasGridFit'
  $g.Clear([Drawing.Color]::White)
  @{B=$b;G=$g}
}
function Save-Canvas($c,$name){ $c.G.Dispose(); $c.B.Save((Join-Path $Out "$name.png"),[Drawing.Imaging.ImageFormat]::Png); $c.B.Dispose() }
# Все линии одной толщины 0,42 мм (СТП, п. 3.2.3), стрелки с развалом 60 градусов (СТП, п. 3.12.2).
function Pen-Of($w=5,[switch]$Dash,[switch]$Arrow){
  $p = New-Object Drawing.Pen $black,5
  if($Dash){ $p.DashStyle='Dash' }
  if($Arrow){ $p.CustomEndCap = New-Object Drawing.Drawing2D.AdjustableArrowCap 4,3.46,$true }
  $p
}
function Text($g,$s,$x,$y,$w,$h,$al='Center',$bold=$false,$size=$FS){
  $st = if($bold){[Drawing.FontStyle]::Bold}else{[Drawing.FontStyle]::Regular}
  $f = New-Object Drawing.Font 'Arial',$size,$st,([Drawing.GraphicsUnit]::Pixel)
  $sf = New-Object Drawing.StringFormat
  $sf.Alignment=$al; $sf.LineAlignment='Center'
  $g.DrawString($s,$f,[Drawing.Brushes]::Black,(New-Object Drawing.RectangleF $x,$y,$w,$h),$sf)
  $f.Dispose(); $sf.Dispose()
}
function Rounded($g,$x,$y,$w,$h,$r,$color=$null){
  $pa = New-Object Drawing.Drawing2D.GraphicsPath
  $pa.AddArc($x,$y,2*$r,2*$r,180,90); $pa.AddArc($x+$w-2*$r,$y,2*$r,2*$r,270,90)
  $pa.AddArc($x+$w-2*$r,$y+$h-2*$r,2*$r,2*$r,0,90); $pa.AddArc($x,$y+$h-2*$r,2*$r,2*$r,90,90); $pa.CloseFigure()
  if($color){ $br=New-Object Drawing.SolidBrush $color; $g.FillPath($br,$pa); $br.Dispose() }
  $g.DrawPath((Pen-Of 4),$pa); $pa.Dispose()
}
function Line($g,$x1,$y1,$x2,$y2,[switch]$Arrow,[switch]$Dash){
  $p = Pen-Of 4 -Arrow:$Arrow -Dash:$Dash
  $g.DrawLine($p,[single]$x1,[single]$y1,[single]$x2,[single]$y2); $p.Dispose()
}
function Actor($g,$cx,$top,$name){
  $p = Pen-Of 5
  $g.DrawEllipse($p,$cx-35,$top,70,70)
  $g.DrawLine($p,$cx,$top+70,$cx,$top+190)
  $g.DrawLine($p,$cx-70,$top+110,$cx+70,$top+110)
  $g.DrawLine($p,$cx,$top+190,$cx-60,$top+280)
  $g.DrawLine($p,$cx,$top+190,$cx+60,$top+280)
  $p.Dispose()
  Text $g $name ($cx-230) ($top-80) 460 75
}
function Oval($g,$x,$y,$w,$h,$s){
  $br=New-Object Drawing.SolidBrush $fill; $g.FillEllipse($br,$x,$y,$w,$h); $br.Dispose()
  $p=Pen-Of 4; $g.DrawEllipse($p,$x,$y,$w,$h); $p.Dispose()
  Text $g $s ($x+55) $y ($w-110) $h
}

# ---------- Рисунок: варианты использования ----------
$c = New-Canvas 1950 1930; $g=$c.G
$g.DrawRectangle((Pen-Of 5),400,90,1100,1770)
Text $g 'Система «ПаркУчёт»' 400 10 1100 80 'Center' $true
$ov = @(
 @(460,120,1010,210,'Регистрация въезда'),
 @(460,360,1010,210,'Оформление выезда'),
 @(760,690,710,210,'Расчёт стоимости'),
 @(460,1000,800,190,'Просмотр свободных мест'),
 @(460,1210,800,190,'Просмотр истории'),
 @(460,1420,800,190,'Аннулирование записи'),
 @(620,1630,850,210,'Настройка тарифа и вместимости'))
foreach($o in $ov){ Oval $g $o[0] $o[1] $o[2] $o[3] $o[4] }
Actor $g 150 620 'Оператор'
Actor $g 1730 1625 'Администратор'
# линии связи только из горизонтальных и вертикальных отрезков (СТП, п. 3.2.2)
Line $g 220 730 330 730
Line $g 330 225 330 1515
foreach($y in 225,465,1095,1305,1515){ Line $g 330 $y 460 $y }
Line $g 1660 1735 1470 1735
# include
Line $g 1115 570 1115 690 -Arrow -Dash
Text $g '«include»' 1130 590 330 80 'Near'
Save-Canvas $c 'use-cases'

# ---------- Рисунок: действия при оформлении выезда ----------
# Схема алгоритма по ГОСТ 19.701-90 (СТП, п. 3.12): символ a = 20 мм, b = 2a; поток сверху вниз и слева направо
# без стрелок, в остальных случаях со стрелкой; признаки решения над линией или справа от неё.
$c = New-Canvas 1950 2690; $g=$c.G
$cx=650; $A=236; $B=472
function Proc($g,$x,$y,$s){ $g.DrawRectangle((Pen-Of),$x,$y,$B,$A); Text $g $s ($x+10) $y ($B-20) $A }
function Term($g,$y,$s){ Rounded $g ($cx-$B/2) $y $B ($A/2) ($A/4); Text $g $s ($cx-$B/2) $y $B ($A/2) }
function Dia($g,$yt,$s){
  $cy=$yt+$A/2
  $pts=[Drawing.Point[]]@((New-Object Drawing.Point $cx,$yt),(New-Object Drawing.Point ($cx+$B/2),$cy),(New-Object Drawing.Point $cx,($yt+$A)),(New-Object Drawing.Point ($cx-$B/2),$cy))
  $g.DrawPolygon((Pen-Of),$pts); Text $g $s ($cx-170) ($cy-75) 340 150
}
$px=$cx-$B/2; $gap=50; $gapD=85
$y=8; Term $g $y 'Начало'; $y+=$A/2
Line $g $cx $y $cx ($y+$gap); $y+=$gap
Proc $g $px $y 'Выбрать автомобиль'; $y+=$A
Line $g $cx $y $cx ($y+$gap); $y+=$gap
$yTime=$y+$A/2
Proc $g $px $y 'Указать время выезда'; $y+=$A
Line $g $cx $y $cx ($y+$gap); $y+=$gap
$yD1=$y+$A/2
Dia $g $y "Время`nверно?"; $y+=$A
Line $g $cx $y $cx ($y+$gapD); Text $g 'Да' ($cx+12) ($y+5) 120 72 'Near'; $y+=$gapD
Proc $g $px $y 'Рассчитать стоимость'; $y+=$A
Line $g $cx $y $cx ($y+$gap); $y+=$gap
Proc $g $px $y 'Показать стоимость'; $y+=$A
Line $g $cx $y $cx ($y+$gap); $y+=$gap
$yD2=$y+$A/2
Dia $g $y "Оплата`nпринята?"; $y+=$A
Line $g $cx $y $cx ($y+$gapD); Text $g 'Да' ($cx+12) ($y+5) 120 72 'Near'; $y+=$gapD
Proc $g $px $y 'Завершить сессию'; $y+=$A
Line $g $cx $y $cx ($y+$gap); $y+=$gap
Proc $g $px $y 'Обновить свободные места'; $y+=$A
Line $g $cx $y $cx ($y+$gap); $y+=$gap
$yEnd=$y+$A/4
Term $g $y 'Конец'
# ветвь «Нет» первого решения: вывод ошибки и возврат к вводу времени
$xE=1320
Line $g ($cx+$B/2) $yD1 $xE $yD1
Text $g 'Нет' ($cx+$B/2+15) ($yD1-72) 130 72 'Near'
Proc $g $xE ($yD1-$A/2) 'Показать ошибку времени'
Line $g ($xE+$B/2) ($yD1-$A/2) ($xE+$B/2) $yTime
Line $g ($xE+$B/2) $yTime ($cx+$B/2) $yTime -Arrow
# ветвь «Нет» второго решения: автомобиль остаётся на стоянке
Line $g ($cx+$B/2) $yD2 ($xE+$B/2) $yD2
Text $g 'Нет' ($cx+$B/2+15) ($yD2-72) 130 72 'Near'
Line $g ($xE+$B/2) $yD2 ($xE+$B/2) $yEnd
Line $g ($xE+$B/2) $yEnd ($cx+$B/2) $yEnd -Arrow
Save-Canvas $c 'exit-flow'

# ---------- Рисунок: состояния сессии ----------
$c = New-Canvas 1950 1370; $g=$c.G
function St($g,$x,$y,$w,$h,$s){ Rounded $g $x $y $w $h 50 $fill; Text $g $s $x $y $w $h }
$g.FillEllipse([Drawing.Brushes]::Black,40,110,70,70)
Line $g 110 145 300 145 -Arrow
St $g 300 60 560 170 'Новая'
Line $g 860 145 1120 145 -Arrow
Text $g "въезд" 870 60 240 80
St $g 1120 60 780 170 'На стоянке'
Line $g 1320 230 1320 650 -Arrow
Text $g "начат`nвыезд" 1335 360 300 150 'Near'
Line $g 1720 650 1720 230 -Arrow
Text $g "отмена" 1730 370 220 90 'Near'
St $g 1120 650 780 170 'Расчёт стоимости'
Line $g 1510 820 1510 1180 -Arrow
Text $g "оплата`nпринята" 1525 920 300 150 'Near'
St $g 1120 1180 780 170 'Завершена'
Line $g 1160 230 1160 460
Line $g 1160 460 490 460
Line $g 490 460 490 1180 -Arrow
Text $g "аннулирование" 500 365 600 90 'Near'
St $g 100 1180 780 170 'Аннулирована'
Save-Canvas $c 'session-states'

# ---------- Общие элементы макетов ----------
function Win($g,$w,$h,$title,$tag){
  Text $g $tag 0 0 $w 75 'Near' $true
  $g.DrawRectangle((Pen-Of 6),3,85,$w-6,$h-88)
  $br=New-Object Drawing.SolidBrush $dark; $g.FillRectangle($br,6,88,$w-12,90); $br.Dispose()
  Line $g 3 178 ($w-3) 178
  Text $g $title 25 88 ($w-160) 90 'Near' $true
  Text $g '×' ($w-100) 88 80 90
}
function Fld($g,$x,$y,$w,$s){ $g.DrawRectangle((Pen-Of 3),$x,$y,$w,90); Text $g $s ($x+15) $y ($w-20) 90 'Near' }
function Btn($g,$x,$y,$w,$s,$h=100){ Rounded $g $x $y $w $h 16 $fill; Text $g $s $x $y $w $h }
function Grid($g,$x,$y,$cols,$rows,$rh=85){
  $tw=($cols|ForEach-Object{$_[0]}|Measure-Object -Sum).Sum
  $th=$rh*(1+$rows.Count)
  $br=New-Object Drawing.SolidBrush $fill; $g.FillRectangle($br,$x,$y,$tw,$rh); $br.Dispose()
  $g.DrawRectangle((Pen-Of 3),$x,$y,$tw,$th)
  $cx=$x
  foreach($cl in $cols){ Text $g $cl[1] $cx $y $cl[0] $rh; if($cx -gt $x){ Line $g $cx $y $cx ($y+$th) }; $cx+=$cl[0] }
  $ry=$y+$rh
  foreach($r in $rows){
    Line $g $x $ry ($x+$tw) $ry
    $cx=$x; for($i=0;$i -lt $cols.Count;$i++){ Text $g $r[$i] ($cx+10) $ry ($cols[$i][0]-15) $rh 'Near'; $cx+=$cols[$i][0] }
    $ry+=$rh
  }
}

# ---------- Макет: главное окно ----------
$c = New-Canvas 1950 1580; $g=$c.G
Win $g 1950 1580 'ПаркУчёт' 'part mainWindow : Window'
# места
$bx=@(30,660,1290); $bt=@('Всего мест: 50','Занято: 12','Свободно: 38')
for($i=0;$i -lt 3;$i++){ $g.DrawRectangle((Pen-Of 4),$bx[$i],205,630,100); Text $g $bt[$i] $bx[$i] 205 630 100 'Center' ($i -eq 2) }
# форма въезда
Text $g 'Въезд автомобиля' 30 320 900 80 'Near' $true
Text $g 'Госномер' 30 400 430 90 'Near'
Fld $g 460 400 530 '1234 AB-7'
Text $g 'Марка' 1030 400 240 90 'Near'
Fld $g 1240 400 680 'Volkswagen'
Text $g 'Время въезда' 30 510 430 90 'Near'
Fld $g 460 510 530 '01.11.2026 19:45'
Btn $g 1030 505 890 'Зарегистрировать въезд'
# список
Text $g 'Автомобили на стоянке' 30 630 1000 80 'Near' $true
Text $g 'Поиск' 1040 630 240 80 'Near'
Fld $g 1290 625 630 '1234'
Grid $g 30 730 @(@(400,'Госномер'),@(400,'Марка'),@(550,'Въезд'),@(540,'Длительность')) @(
  @('1234 AB-7','Volkswagen','01.11.2026 08:15','11 ч 30 мин'),
  @('5678 KI-5','Renault','01.11.2026 09:40','10 ч 05 мин'),
  @('0042 MH-7','Geely','01.11.2026 12:05','7 ч 40 мин'),
  @('7310 EX-3','Lada','01.11.2026 17:20','2 ч 25 мин'))
Btn $g 30 1190 930 'Оформить выезд'
Btn $g 990 1190 930 'Аннулировать'
Btn $g 30 1320 930 'История'
Btn $g 990 1320 930 'Настройки'
Text $g 'Тариф: 2,00 BYN за час, льготный период 10 мин' 30 1460 1890 90 'Near'
Save-Canvas $c 'mockup-main'

# ---------- Макет: выезд ----------
$c = New-Canvas 1950 1020; $g=$c.G
Win $g 1950 1020 'Оформление выезда' 'part exitWindow : Window'
$rows=@(@('Госномер','1234 AB-7'),@('Марка','Volkswagen'),@('Въезд','01.11.2026 08:15'))
$y=200
foreach($r in $rows){ Text $g $r[0] 40 $y 600 85 'Near'; Text $g $r[1] 680 $y 1200 85 'Near'; $y+=85 }
Text $g 'Выезд' 40 $y 600 95 'Near'; Fld $g 680 ($y+2) 700 '01.11.2026 19:45'; $y+=105
Text $g 'Длительность' 40 $y 600 85 'Near'; Text $g '11 ч 30 мин (12 начатых часов)' 680 $y 1250 85 'Near'; $y+=85
Text $g 'Тариф' 40 $y 600 85 'Near'; Text $g '2,00 BYN за час, льготно 10 мин' 680 $y 1250 85 'Near'; $y+=95
$br=New-Object Drawing.SolidBrush $fill; $g.FillRectangle($br,40,$y,1870,120); $br.Dispose(); $g.DrawRectangle((Pen-Of 4),40,$y,1870,120)
Text $g 'К оплате: 24,00 BYN' 40 $y 1870 120 'Center' $true 72
$y+=150
Btn $g 40 $y 930 'Оплата принята'
Btn $g 1000 $y 910 'Отмена'
Save-Canvas $c 'mockup-exit'

# ---------- Макет: история ----------
$c = New-Canvas 1950 1020; $g=$c.G
Win $g 1950 1020 'История стоянок' 'part historyWindow : Window'
Text $g 'С' 30 205 60 90 'Near'; Fld $g 90 205 350 '01.11.2026'
Text $g 'По' 450 205 120 90 'Near'; Fld $g 560 205 350 '01.11.2026'
Text $g 'Номер' 920 205 240 90 'Near'; Fld $g 1150 205 410 '1234'
Btn $g 1580 200 340 'Найти'
Grid $g 30 320 @(@(380,'Госномер'),@(340,'Въезд'),@(340,'Выезд'),@(260,'Сумма'),@(570,'Статус')) @(
  @('1234 AB-7','08:15','19:45','24,00','Завершена'),
  @('5678 KI-5','09:40','11:00','4,00','Завершена'),
  @('0042 MH-7','12:05','12:12','0,00','Завершена'),
  @('7310 EX-3','17:20','17:21','0,00','Аннулирована'))
Text $g 'Записей: 4' 30 770 900 90 'Near'
Text $g 'Итого: 28,00 BYN' 1000 770 920 90 'Far' $true
Btn $g 30 880 930 'Выгрузить в CSV'
Btn $g 990 880 930 'Закрыть'
Save-Canvas $c 'mockup-history'

# ---------- Макет: настройки ----------
$c = New-Canvas 1950 830; $g=$c.G
Win $g 1950 830 'Настройки' 'part settingsWindow : Window'
$rows=@(@('Вместимость, мест','50'),@('Ставка за час, BYN','2,00'),@('Льготный период, мин','10'))
$y=210
foreach($r in $rows){ Text $g $r[0] 40 $y 1050 90 'Near'; Fld $g 1100 $y 800 $r[1]; $y+=115 }
Text $g 'Новый тариф действует для следующих въездов' 40 $y 1870 80 'Near'
$y+=95
Btn $g 40 $y 930 'Сохранить'
Btn $g 1000 $y 910 'Отмена'
Save-Canvas $c 'mockup-settings'
"Готово: $Out"
