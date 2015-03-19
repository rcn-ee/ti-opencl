/*
 *  Do not modify this file; it is automatically 
 *  generated and any modifications will be overwritten.
 *
 * @(#) xdc-y25
 */
import java.util.*;
import org.mozilla.javascript.*;
import xdc.services.intern.xsr.*;
import xdc.services.spec.Session;

public class platform_evm6678
{
    static final String VERS = "@(#) xdc-y25\n";

    static final Proto.Elm $$T_Bool = Proto.Elm.newBool();
    static final Proto.Elm $$T_Num = Proto.Elm.newNum();
    static final Proto.Elm $$T_Str = Proto.Elm.newStr();
    static final Proto.Elm $$T_Obj = Proto.Elm.newObj();

    static final Proto.Fxn $$T_Met = new Proto.Fxn(null, null, 0, -1, false);
    static final Proto.Map $$T_Map = new Proto.Map($$T_Obj);
    static final Proto.Arr $$T_Vec = new Proto.Arr($$T_Obj);

    static final XScriptO $$DEFAULT = Value.DEFAULT;
    static final Object $$UNDEF = Undefined.instance;

    static final Proto.Obj $$Package = (Proto.Obj)Global.get("$$Package");
    static final Proto.Obj $$Module = (Proto.Obj)Global.get("$$Module");
    static final Proto.Obj $$Instance = (Proto.Obj)Global.get("$$Instance");
    static final Proto.Obj $$Params = (Proto.Obj)Global.get("$$Params");

    static final Object $$objFldGet = Global.get("$$objFldGet");
    static final Object $$objFldSet = Global.get("$$objFldSet");
    static final Object $$proxyGet = Global.get("$$proxyGet");
    static final Object $$proxySet = Global.get("$$proxySet");
    static final Object $$delegGet = Global.get("$$delegGet");
    static final Object $$delegSet = Global.get("$$delegSet");

    Scriptable xdcO;
    Session ses;
    Value.Obj om;

    boolean isROV;
    boolean isCFG;

    Proto.Obj pkgP;
    Value.Obj pkgV;

    ArrayList<Object> imports = new ArrayList<Object>();
    ArrayList<Object> loggables = new ArrayList<Object>();
    ArrayList<Object> mcfgs = new ArrayList<Object>();
    ArrayList<Object> icfgs = new ArrayList<Object>();
    ArrayList<String> inherits = new ArrayList<String>();
    ArrayList<Object> proxies = new ArrayList<Object>();
    ArrayList<Object> sizes = new ArrayList<Object>();
    ArrayList<Object> tdefs = new ArrayList<Object>();

    void $$IMPORTS()
    {
        Global.callFxn("loadPackage", xdcO, "xdc");
        Global.callFxn("loadPackage", xdcO, "xdc.corevers");
        Global.callFxn("loadPackage", xdcO, "ti.platforms.generic");
        Global.callFxn("loadPackage", xdcO, "xdc.platform");
    }

    void $$OBJECTS()
    {
        pkgP = (Proto.Obj)om.bind("platform.evm6678.Package", new Proto.Obj());
        pkgV = (Value.Obj)om.bind("platform.evm6678", new Value.Obj("platform.evm6678", pkgP));
    }

    void Platform$$OBJECTS()
    {
        Proto.Obj po, spo;
        Value.Obj vo;

        po = (Proto.Obj)om.bind("platform.evm6678.Platform.Module", new Proto.Obj());
        vo = (Value.Obj)om.bind("platform.evm6678.Platform", new Value.Obj("platform.evm6678.Platform", po));
        pkgV.bind("Platform", vo);
        // decls 
        om.bind("platform.evm6678.Platform.Board", om.findStrict("xdc.platform.IPlatform.Board", "platform.evm6678"));
        om.bind("platform.evm6678.Platform.Memory", om.findStrict("xdc.platform.IPlatform.Memory", "platform.evm6678"));
        // insts 
        Object insP = om.bind("platform.evm6678.Platform.Instance", new Proto.Obj());
        po = (Proto.Obj)om.bind("platform.evm6678.Platform$$Object", new Proto.Obj());
        Object objP = om.bind("platform.evm6678.Platform.Object", new Proto.Str(po, true));
        po = (Proto.Obj)om.bind("platform.evm6678.Platform$$Params", new Proto.Obj());
        om.bind("platform.evm6678.Platform.Params", new Proto.Str(po, true));
    }

    void Platform$$CONSTS()
    {
        // module Platform
    }

    void Platform$$CREATES()
    {
        Proto.Fxn fxn;
        StringBuilder sb;

        fxn = (Proto.Fxn)om.bind("platform.evm6678.Platform$$create", new Proto.Fxn(om.findStrict("platform.evm6678.Platform.Module", "platform.evm6678"), om.findStrict("platform.evm6678.Platform.Instance", "platform.evm6678"), 3, 2, false));
                fxn.addArg(0, "name", $$T_Str, $$UNDEF);
                fxn.addArg(1, "args", $$T_Obj, $$UNDEF);
                fxn.addArg(2, "__params", (Proto)om.findStrict("platform.evm6678.Platform.Params", "platform.evm6678"), Global.newObject());
        sb = new StringBuilder();
        sb.append("platform$evm6678$Platform$$create = function( name, args, __params ) {\n");
            sb.append("var __mod = xdc.om['platform.evm6678.Platform'];\n");
            sb.append("var __inst = xdc.om['platform.evm6678.Platform.Instance'].$$make();\n");
            sb.append("__inst.$$bind('$package', xdc.om['platform.evm6678']);\n");
            sb.append("__inst.$$bind('$index', __mod.$instances.length);\n");
            sb.append("__inst.$$bind('$category', 'Instance');\n");
            sb.append("__inst.$$bind('$args', {name:name, args:args});\n");
            sb.append("__inst.$$bind('$module', __mod);\n");
            sb.append("__mod.$instances.$add(__inst);\n");
            sb.append("__inst.externalMemoryMap = __mod.PARAMS.externalMemoryMap;\n");
            sb.append("__inst.customMemoryMap = __mod.PARAMS.customMemoryMap;\n");
            sb.append("__inst.renameMap = __mod.PARAMS.renameMap;\n");
            sb.append("__inst.dataMemory = __mod.PARAMS.dataMemory;\n");
            sb.append("__inst.codeMemory = __mod.PARAMS.codeMemory;\n");
            sb.append("__inst.stackMemory = __mod.PARAMS.stackMemory;\n");
            sb.append("__inst.sectMap = __mod.PARAMS.sectMap;\n");
            sb.append("for (var __p in __params) __inst[__p] = __params[__p];\n");
            sb.append("var save = xdc.om.$curpkg;\n");
            sb.append("xdc.om.$$bind('$curpkg', __mod.$package.$name);\n");
            sb.append("__mod.instance$meta$init.$fxn.apply(__inst, [name, args]);\n");
            sb.append("xdc.om.$$bind('$curpkg', save);\n");
            sb.append("__inst.$$bless();\n");
            sb.append("return __inst;\n");
        sb.append("}\n");
        Global.eval(sb.toString());
        fxn = (Proto.Fxn)om.bind("platform.evm6678.Platform$$construct", new Proto.Fxn(om.findStrict("platform.evm6678.Platform.Module", "platform.evm6678"), null, 4, 2, false));
                fxn.addArg(0, "__obj", (Proto)om.findStrict("platform.evm6678.Platform$$Object", "platform.evm6678"), null);
                fxn.addArg(1, "name", $$T_Str, $$UNDEF);
                fxn.addArg(2, "args", $$T_Obj, $$UNDEF);
                fxn.addArg(3, "__params", (Proto)om.findStrict("platform.evm6678.Platform.Params", "platform.evm6678"), Global.newObject());
        sb = new StringBuilder();
        sb.append("platform$evm6678$Platform$$construct = function( __obj, name, args, __params ) {\n");
            sb.append("var __mod = xdc.om['platform.evm6678.Platform'];\n");
            sb.append("var __inst = __obj;\n");
            sb.append("__inst.$$bind('$args', {name:name, args:args});\n");
            sb.append("__inst.$$bind('$module', __mod);\n");
            sb.append("__mod.$objects.$add(__inst);\n");
            sb.append("__inst.externalMemoryMap = __mod.PARAMS.externalMemoryMap;\n");
            sb.append("__inst.customMemoryMap = __mod.PARAMS.customMemoryMap;\n");
            sb.append("__inst.renameMap = __mod.PARAMS.renameMap;\n");
            sb.append("__inst.dataMemory = __mod.PARAMS.dataMemory;\n");
            sb.append("__inst.codeMemory = __mod.PARAMS.codeMemory;\n");
            sb.append("__inst.stackMemory = __mod.PARAMS.stackMemory;\n");
            sb.append("__inst.sectMap = __mod.PARAMS.sectMap;\n");
            sb.append("for (var __p in __params) __inst[__p] = __params[__p];\n");
            sb.append("__inst.$$bless();\n");
            sb.append("return null;\n");
        sb.append("}\n");
        Global.eval(sb.toString());
    }

    void Platform$$FUNCTIONS()
    {
        Proto.Fxn fxn;

    }

    void Platform$$SIZES()
    {
    }

    void Platform$$TYPES()
    {
        Scriptable cap;
        Proto.Obj po;
        Proto.Str ps;
        Proto.Typedef pt;
        Object fxn;

        cap = (Scriptable)Global.callFxn("loadCapsule", xdcO, "platform/evm6678/Platform.xs");
        om.bind("platform.evm6678.Platform$$capsule", cap);
        po = (Proto.Obj)om.findStrict("platform.evm6678.Platform.Module", "platform.evm6678");
        po.init("platform.evm6678.Platform.Module", om.findStrict("xdc.platform.IPlatform.Module", "platform.evm6678"));
                po.addFld("$hostonly", $$T_Num, 1, "r");
        po.addFld("CPU", (Proto)om.findStrict("ti.platforms.generic.Platform.Instance", "platform.evm6678"), $$UNDEF, "wh");
                po.addFxn("create", (Proto.Fxn)om.findStrict("platform.evm6678.Platform$$create", "platform.evm6678"), Global.get("platform$evm6678$Platform$$create"));
                po.addFxn("construct", (Proto.Fxn)om.findStrict("platform.evm6678.Platform$$construct", "platform.evm6678"), Global.get("platform$evm6678$Platform$$construct"));
        fxn = Global.get(cap, "module$use");
        if (fxn != null) om.bind("platform.evm6678.Platform$$module$use", true);
        if (fxn != null) po.addFxn("module$use", $$T_Met, fxn);
        fxn = Global.get(cap, "module$meta$init");
        if (fxn != null) om.bind("platform.evm6678.Platform$$module$meta$init", true);
        if (fxn != null) po.addFxn("module$meta$init", $$T_Met, fxn);
        fxn = Global.get(cap, "instance$meta$init");
        if (fxn != null) om.bind("platform.evm6678.Platform$$instance$meta$init", true);
        if (fxn != null) po.addFxn("instance$meta$init", $$T_Met, fxn);
        fxn = Global.get(cap, "module$validate");
        if (fxn != null) om.bind("platform.evm6678.Platform$$module$validate", true);
        if (fxn != null) po.addFxn("module$validate", $$T_Met, fxn);
        po = (Proto.Obj)om.findStrict("platform.evm6678.Platform.Instance", "platform.evm6678");
        po.init("platform.evm6678.Platform.Instance", om.findStrict("xdc.platform.IPlatform.Instance", "platform.evm6678"));
                po.addFld("$hostonly", $$T_Num, 1, "r");
        po.addFld("codeMemory", $$T_Str, "DDR3_STATIC", "wh");
        po.addFld("dataMemory", $$T_Str, "MSMCSRAM_NOCACHE", "wh");
        po.addFld("stackMemory", $$T_Str, "L2SRAM1", "wh");
                fxn = Global.get(cap, "getCpuDataSheet");
                if (fxn != null) po.addFxn("getCpuDataSheet", (Proto.Fxn)om.findStrict("xdc.platform.IPlatform$$getCpuDataSheet", "platform.evm6678"), fxn);
                fxn = Global.get(cap, "getCreateArgs");
                if (fxn != null) po.addFxn("getCreateArgs", (Proto.Fxn)om.findStrict("xdc.platform.IPlatform$$getCreateArgs", "platform.evm6678"), fxn);
                fxn = Global.get(cap, "getExeContext");
                if (fxn != null) po.addFxn("getExeContext", (Proto.Fxn)om.findStrict("xdc.platform.IPlatform$$getExeContext", "platform.evm6678"), fxn);
                fxn = Global.get(cap, "getExecCmd");
                if (fxn != null) po.addFxn("getExecCmd", (Proto.Fxn)om.findStrict("xdc.platform.IPlatform$$getExecCmd", "platform.evm6678"), fxn);
                fxn = Global.get(cap, "getLinkTemplate");
                if (fxn != null) po.addFxn("getLinkTemplate", (Proto.Fxn)om.findStrict("xdc.platform.IPlatform$$getLinkTemplate", "platform.evm6678"), fxn);
        po = (Proto.Obj)om.findStrict("platform.evm6678.Platform$$Params", "platform.evm6678");
        po.init("platform.evm6678.Platform.Params", om.findStrict("xdc.platform.IPlatform$$Params", "platform.evm6678"));
                po.addFld("$hostonly", $$T_Num, 1, "r");
        po.addFld("codeMemory", $$T_Str, "DDR3_STATIC", "wh");
        po.addFld("dataMemory", $$T_Str, "MSMCSRAM_NOCACHE", "wh");
        po.addFld("stackMemory", $$T_Str, "L2SRAM1", "wh");
        po = (Proto.Obj)om.findStrict("platform.evm6678.Platform$$Object", "platform.evm6678");
        po.init("platform.evm6678.Platform.Object", om.findStrict("platform.evm6678.Platform.Instance", "platform.evm6678"));
                fxn = Global.get(cap, "getCpuDataSheet");
                if (fxn != null) po.addFxn("getCpuDataSheet", (Proto.Fxn)om.findStrict("xdc.platform.IPlatform$$getCpuDataSheet", "platform.evm6678"), fxn);
                fxn = Global.get(cap, "getCreateArgs");
                if (fxn != null) po.addFxn("getCreateArgs", (Proto.Fxn)om.findStrict("xdc.platform.IPlatform$$getCreateArgs", "platform.evm6678"), fxn);
                fxn = Global.get(cap, "getExeContext");
                if (fxn != null) po.addFxn("getExeContext", (Proto.Fxn)om.findStrict("xdc.platform.IPlatform$$getExeContext", "platform.evm6678"), fxn);
                fxn = Global.get(cap, "getExecCmd");
                if (fxn != null) po.addFxn("getExecCmd", (Proto.Fxn)om.findStrict("xdc.platform.IPlatform$$getExecCmd", "platform.evm6678"), fxn);
                fxn = Global.get(cap, "getLinkTemplate");
                if (fxn != null) po.addFxn("getLinkTemplate", (Proto.Fxn)om.findStrict("xdc.platform.IPlatform$$getLinkTemplate", "platform.evm6678"), fxn);
    }

    void Platform$$ROV()
    {
    }

    void $$SINGLETONS()
    {
        pkgP.init("platform.evm6678.Package", (Proto.Obj)om.findStrict("xdc.IPackage.Module", "platform.evm6678"));
        pkgP.bind("$capsule", $$UNDEF);
        pkgV.init2(pkgP, "platform.evm6678", Value.DEFAULT, false);
        pkgV.bind("$name", "platform.evm6678");
        pkgV.bind("$category", "Package");
        pkgV.bind("$$qn", "platform.evm6678.");
        pkgV.bind("$vers", Global.newArray());
        Value.Map atmap = (Value.Map)pkgV.getv("$attr");
        atmap.seal("length");
        imports.clear();
        pkgV.bind("$imports", imports);
        StringBuilder sb = new StringBuilder();
        sb.append("var pkg = xdc.om['platform.evm6678'];\n");
        sb.append("if (pkg.$vers.length >= 3) {\n");
            sb.append("pkg.$vers.push(Packages.xdc.services.global.Vers.getDate(xdc.csd() + '/..'));\n");
        sb.append("}\n");
        sb.append("pkg.build.libraries = [\n");
        sb.append("];\n");
        sb.append("pkg.build.libDesc = [\n");
        sb.append("];\n");
        Global.eval(sb.toString());
    }

    void Platform$$SINGLETONS()
    {
        Proto.Obj po;
        Value.Obj vo;

        vo = (Value.Obj)om.findStrict("platform.evm6678.Platform", "platform.evm6678");
        po = (Proto.Obj)om.findStrict("platform.evm6678.Platform.Module", "platform.evm6678");
        vo.init2(po, "platform.evm6678.Platform", $$DEFAULT, false);
        vo.bind("Module", po);
        vo.bind("$category", "Module");
        vo.bind("$capsule", om.findStrict("platform.evm6678.Platform$$capsule", "platform.evm6678"));
        vo.bind("Instance", om.findStrict("platform.evm6678.Platform.Instance", "platform.evm6678"));
        vo.bind("Params", om.findStrict("platform.evm6678.Platform.Params", "platform.evm6678"));
        vo.bind("PARAMS", ((Proto.Str)om.findStrict("platform.evm6678.Platform.Params", "platform.evm6678")).newInstance());
        vo.bind("$package", om.findStrict("platform.evm6678", "platform.evm6678"));
        tdefs.clear();
        proxies.clear();
        mcfgs.clear();
        icfgs.clear();
        inherits.clear();
        vo.bind("Board", om.findStrict("xdc.platform.IPlatform.Board", "platform.evm6678"));
        tdefs.add(om.findStrict("xdc.platform.IPlatform.Board", "platform.evm6678"));
        vo.bind("Memory", om.findStrict("xdc.platform.IPlatform.Memory", "platform.evm6678"));
        tdefs.add(om.findStrict("xdc.platform.IPlatform.Memory", "platform.evm6678"));
        vo.bind("MemoryMap", om.findStrict("xdc.platform.IPlatform.MemoryMap", "platform.evm6678"));
        vo.bind("$$tdefs", Global.newArray(tdefs.toArray()));
        vo.bind("$$proxies", Global.newArray(proxies.toArray()));
        vo.bind("$$mcfgs", Global.newArray(mcfgs.toArray()));
        vo.bind("$$icfgs", Global.newArray(icfgs.toArray()));
        inherits.add("xdc.platform");
        vo.bind("$$inherits", Global.newArray(inherits.toArray()));
        ((Value.Arr)pkgV.getv("$modules")).add(vo);
        ((Value.Arr)om.findStrict("$modules", "platform.evm6678")).add(vo);
        vo.bind("$$instflag", 1);
        vo.bind("$$iobjflag", 1);
        vo.bind("$$sizeflag", 1);
        vo.bind("$$dlgflag", 0);
        vo.bind("$$iflag", 1);
        vo.bind("$$romcfgs", "|");
        vo.bind("$$nortsflag", 0);
        Proto.Str ps = (Proto.Str)vo.find("Module_State");
        if (ps != null) vo.bind("$object", ps.newInstance());
        vo.bind("$$meta_iobj", om.has("platform.evm6678.Platform$$instance$static$init", null) ? 1 : 0);
        vo.bind("$$fxntab", Global.newArray());
        vo.bind("$$logEvtCfgs", Global.newArray());
        vo.bind("$$errorDescCfgs", Global.newArray());
        vo.bind("$$assertDescCfgs", Global.newArray());
        Value.Map atmap = (Value.Map)vo.getv("$attr");
        atmap.seal("length");
        vo.bind("Object", om.findStrict("platform.evm6678.Platform.Object", "platform.evm6678"));
        pkgV.bind("Platform", vo);
        ((Value.Arr)pkgV.getv("$unitNames")).add("Platform");
    }

    void $$INITIALIZATION()
    {
        Value.Obj vo;

        if (isCFG) {
        }//isCFG
        Global.callFxn("module$meta$init", (Scriptable)om.findStrict("platform.evm6678.Platform", "platform.evm6678"));
        vo = (Value.Obj)om.findStrict("platform.evm6678.Platform", "platform.evm6678");
        Global.put(vo, "CPU", Global.callFxn("create", (Scriptable)om.find("ti.platforms.generic.Platform"), "CPU", Global.newObject("clockRate", 1000L, "catalogName", "ti.catalog.c6000", "deviceName", "TMS320C6678", "customMemoryMap", Global.newArray(new Object[]{Global.newArray(new Object[]{"L2SRAM0", Global.newObject("name", "L2SRAM0", "base", 0x00800000L, "len", 0x80L)}), Global.newArray(new Object[]{"L2SRAM1", Global.newObject("name", "L2SRAM1", "base", 0x00800080L, "len", 0x2bf80L)}), Global.newArray(new Object[]{"L2SRAM", Global.newObject("name", "L2SRAM", "base", 0x0082c000L, "len", 0x34000L)}), Global.newArray(new Object[]{"MSMCSRAM", Global.newObject("name", "MSMCSRAM", "base", 0x0C000000L, "len", 0x300000L)}), Global.newArray(new Object[]{"DDR3", Global.newObject("name", "DDR3", "base", 0x80000000L, "len", 0x3EE00000L)}), Global.newArray(new Object[]{"MSMCSRAM_NOCACHE", Global.newObject("name", "MSMCSRAM_NOCACHE", "base", 0xBEE00000L, "len", 0x100000L)}), Global.newArray(new Object[]{"DDR3_STATIC", Global.newObject("name", "DDR3_STATIC", "base", 0xBEF00000L, "len", 0x100000L)}), Global.newArray(new Object[]{"DDR3_NOCACHE0", Global.newObject("name", "DDR3_NOCACHE0", "base", 0xBF000000L, "len", 0x800000L)}), Global.newArray(new Object[]{"DDR3_NOCACHE1", Global.newObject("name", "DDR3_NOCACHE1", "base", 0xBF800000L, "len", 0x800000L)})}), "l1DMode", "32k", "l1PMode", "32k", "l2Mode", "128k")));
        Global.callFxn("init", pkgV);
        ((Value.Obj)om.getv("platform.evm6678.Platform")).bless();
        ((Value.Arr)om.findStrict("$packages", "platform.evm6678")).add(pkgV);
    }

    public void exec( Scriptable xdcO, Session ses )
    {
        this.xdcO = xdcO;
        this.ses = ses;
        om = (Value.Obj)xdcO.get("om", null);

        Object o = om.geto("$name");
        String s = o instanceof String ? (String)o : null;
        isCFG = s != null && s.equals("cfg");
        isROV = s != null && s.equals("rov");

        $$IMPORTS();
        $$OBJECTS();
        Platform$$OBJECTS();
        Platform$$CONSTS();
        Platform$$CREATES();
        Platform$$FUNCTIONS();
        Platform$$SIZES();
        Platform$$TYPES();
        if (isROV) {
            Platform$$ROV();
        }//isROV
        $$SINGLETONS();
        Platform$$SINGLETONS();
        $$INITIALIZATION();
    }
}
