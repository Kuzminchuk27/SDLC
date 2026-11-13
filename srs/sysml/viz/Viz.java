import org.omg.sysml.interactive.*;
import java.nio.file.*;
import java.util.*;
// Обработка моделей .sysml эталонной реализацией SysML v2 и построение диаграмм командой viz (аналог %viz в Jupyter).
// Аргументы: <каталог библиотеки> <выходной каталог> <файл.sysml>... затем "--" и задания вида имя;вид;стили;файл
public class Viz {
  public static void main(String[] a) throws Exception {
    SysMLInteractive s = SysMLInteractive.getInstance();
    s.loadLibrary(a[0]);
    Path out = Paths.get(a[1]);
    int i = 2;
    for (; i < a.length && !a[i].equals("--"); i++) {
      String text = Files.readString(Paths.get(a[i]));
      SysMLInteractiveResult r = s.process(text);
      System.out.println("=== " + a[i] + ": " + r.toString().trim());
    }
    List<String> jobs = new ArrayList<>();
    for (i++; i < a.length; i++) if (a[i].startsWith("@")) jobs.addAll(Files.readAllLines(Paths.get(a[i].substring(1)))); else jobs.add(a[i]);
    for (String job : jobs) {
      if (job.isBlank() || job.startsWith("#")) continue;
      String[] t = job.split(";");
      List<String> styles = t[2].isEmpty() ? List.of() : Arrays.asList(t[2].split(","));
      VizResult v = s.viz(List.of(t[0]), List.of(t[1]), styles, List.of());
      if (v.hasException()) { System.out.println("ОШИБКА " + t[0] + ": " + v.formatException()); continue; }
      System.out.println("viz " + t[0] + " [" + t[1] + "] -> " + v.kind);
      if (v.getSVG() != null) Files.writeString(out.resolve(t[3] + ".svg"), v.getSVG());
      if (v.getPlantUML() != null) Files.writeString(out.resolve(t[3] + ".puml"), v.getPlantUML());
      if (v.getText() != null) System.out.println(v.getText());
    }
  }
}
