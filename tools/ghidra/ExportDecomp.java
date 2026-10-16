// Headless Ghidra script: seed functions, apply symbol names, export decompiled C + call graph.
// args: <symbols.txt (splat format)> <out_dir>
// usage: analyzeHeadless <proj_dir> <name> -import X.elf -scriptPath tools/ghidra -postScript ExportDecomp.java symbols/X.txt build/decomp/X -deleteProject
import ghidra.app.decompiler.*;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import java.io.*;
import java.nio.file.*;
import java.util.*;
import java.util.regex.*;

public class ExportDecomp extends GhidraScript {
    public void run() throws Exception {
        String[] a = getScriptArgs();
        Path symPath = Paths.get(a[0]); Path out = Paths.get(a[1]);
        Files.createDirectories(out);
        FunctionManager fm = currentProgram.getFunctionManager();
        SymbolTable st = currentProgram.getSymbolTable();
        // 1. symbols: "name = 0xADDR; // type:func ..." -> create function if missing, set name
        Pattern p = Pattern.compile("^(\\w+)\\s*=\\s*0x([0-9A-Fa-f]+);\\s*//\\s*(.*)$");
        int seeded = 0, named = 0;
        if (Files.exists(symPath)) for (String line : Files.readAllLines(symPath)) {
            Matcher m = p.matcher(line.trim()); if (!m.matches()) continue;
            Address addr = toAddr(Long.parseLong(m.group(2), 16));
            boolean isFunc = m.group(3).contains("type:func");
            if (isFunc) {
                Function f = fm.getFunctionAt(addr);
                if (f == null) { disassemble(addr); f = createFunction(addr, m.group(1)); if (f != null) seeded++; }
                if (f != null && !f.getName().equals(m.group(1))) { f.setName(m.group(1), SourceType.USER_DEFINED); named++; }
            } else { createLabel(addr, m.group(1), true, SourceType.USER_DEFINED); named++; }
        }
        analyzeChanges(currentProgram);
        println("seeded=" + seeded + " named=" + named + " functions=" + fm.getFunctionCount());
        // 2. export
        DecompInterface di = new DecompInterface(); di.openProgram(currentProgram);
        PrintWriter idx = new PrintWriter(Files.newBufferedWriter(out.resolve("index.tsv")));
        idx.println("address\tname\tsize\tcallees\tcallers\tstrings");
        for (Function f : fm.getFunctions(true)) {
            if (f.isExternal() || f.isThunk()) continue;
            DecompileResults r = di.decompileFunction(f, 60, monitor);
            String code = r.decompileCompleted() ? r.getDecompiledFunction().getC() : "/* decompile failed: " + r.getErrorMessage() + " */\n";
            Files.writeString(out.resolve(f.getName() + ".c"), "// " + f.getEntryPoint() + " size=" + f.getBody().getNumAddresses() + "\n" + code);
            StringJoiner ce = new StringJoiner(","), cr = new StringJoiner(","), ss = new StringJoiner("|");
            for (Function g : f.getCalledFunctions(monitor)) ce.add(g.getName());
            for (Function g : f.getCallingFunctions(monitor)) cr.add(g.getName());
            ReferenceManager rm = currentProgram.getReferenceManager();
            for (AddressIterator it = rm.getReferenceSourceIterator(f.getBody(), true); it.hasNext();) {
                for (Reference ref : rm.getReferencesFrom(it.next())) {
                    Data d = getDataAt(ref.getToAddress());
                    if (d != null && d.hasStringValue()) ss.add(d.getValue().toString().replace("\t"," ").replace("\n","\\n"));
                }
            }
            idx.println(f.getEntryPoint() + "\t" + f.getName() + "\t" + f.getBody().getNumAddresses() + "\t" + ce + "\t" + cr + "\t" + ss);
        }
        idx.close(); di.dispose();
        println("exported to " + out);
    }
}
