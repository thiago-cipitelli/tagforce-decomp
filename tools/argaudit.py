#!/usr/bin/env python3
"""argaudit.py [--all]: calls through K&R prototypes (`f();`) whose argument count differs from f's definition in
the same module. Default: only calls inside `#ifdef NON_MATCHING` drafts (often a missing argument: check the
argument register just before the jal in the original). --all also lists matched code, where the original passes
fewer arguments too (wrappers that forward registers): the port has to give those calls their real arguments.
sanity: tools/argaudit.py --check"""
import re,glob,sys,collections
defs={}
for f in glob.glob("src/*/*.c"):
    mod=f.split('/')[1]
    s=open(f,errors='replace').read()
    for m in re.finditer(r"^[A-Za-z][\w \*]*?\b(\w+)\(([^;{)]*)\)\s*\n\{",s,re.M):
        args=m.group(2).strip()
        n=0 if args in("","void") else args.count(',')+1
        if '...' in args: n=-1
        defs[(mod,m.group(1))]=n
def split_args(s,i):
    # s[i]=='(' ; return list of args and end index
    d=0;cur='';args=[];j=i
    while j<len(s):
        c=s[j]
        if c=='(':
            d+=1
            if d==1: j+=1; continue
        elif c==')':
            d-=1
            if d==0: args.append(cur); return args,j
        elif c==',' and d==1: args.append(cur); cur=''; j+=1; continue
        cur+=c; j+=1
    return None,j
out=collections.defaultdict(list)
for f in sorted(glob.glob("src/*/*.c")):
    mod=f.split('/')[1]
    s=open(f,errors='replace').read()
    s=re.sub(r"/\*.*?\*/",lambda m:re.sub(r"[^\n]"," ",m.group(0)),s,flags=re.S)
    nm=[(m.start(),s.index("#endif",m.start())) for m in re.finditer(r"^#ifdef NON_MATCHING",s,re.M)]
    kr=set(re.findall(r"^[A-Za-z][\w \*]*?\b(\w+)\(\);",s,re.M))
    for name in kr:
        n=defs.get((mod,name))
        if n is None or n<0: continue
        for m in re.finditer(r"\b%s\s*\("%name,s):
            # skip declarations
            line=s[s.rfind('\n',0,m.start())+1:s.find('\n',m.start())]
            if re.match(r"^[A-Za-z][\w \*]*?\b%s\(\);"%name,line): continue
            args,_=split_args(s,m.end()-1)
            if args is None: continue
            k=0 if len(args)==1 and not args[0].strip() else len(args)
            inm=any(a<=m.start()<b for a,b in nm)
            if k!=n and (inm or "--all" in sys.argv):
                out[f].append(f"{name}: {k} args, def has {n}: {line.strip()[:110]}")
if "--check" in sys.argv:
    assert defs.get(("libduel_eng", "duel_0884E398")) == 2 and defs.get(("BOOT", "region_pick_value")) == 1, "definition parser"
    print("argaudit.py: ok"); sys.exit(0)
for f,v in out.items():
    print("==",f)
    for x in v: print("  ",x)
