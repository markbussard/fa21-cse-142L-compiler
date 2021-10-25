import matplotlib.pyplot as plt
import math

import subprocess 
from cfg.cfg import do_cfg
import os
from collections import namedtuple
from CSE142L.cli import cse142L_run
from IPython.display import Image, IFrame

os.environ['CSE142L_LAB'] = 'compiler'
#os.environ['DJR_SERVER'] = 'http://prerelease-dot-cse142l-dev.wl.r.appspot.com/'

columns=["IC", "CPI", "CT", "ET", "cmdlineMHz", "realMHz"] # This is the subset of the data we will pay attention to.

user = None

RenderedCode =namedtuple("RenderedCode", "source asm cfg cfg_counts gprof call_graph")

def shell_cmd(cmd, shell=False, quiet_on_success=False):
    try:
        if not quiet_on_success:
            print(cmd)
        if shell:
            p = subprocess.run(cmd, shell=True, check=True, stderr=subprocess.STDOUT, stdout=subprocess.PIPE)
        else:
            p = subprocess.run(cmd.split(), check=True, stderr=subprocess.STDOUT, stdout=subprocess.PIPE)

        if not quiet_on_success:
            print(p.stdout.decode())
    except subprocess.CalledProcessError as e:
        print(e.output.decode())

    return
    


def do_gprof(exe, gmon="gmon.out", out=None):
    if out is None:
        out = f"{exe}.gprof"
    shell_cmd(f"gprof {exe} > {out}", shell=True, quiet_on_success=True)
    with open(out) as f:
        return HTML(f"<pre>{f.read()}</pre>")
        
def do_call_graph(exe, gmon="gmon.out", root=None, out=None):
    if not root:
        root = "main"
        
    if out is None:
        out = f"{exe}.call_graph.png"
    shell_cmd(f"gprof {exe} | gprof2dot -n0 -e0 -z {root} | dot -Tpng -o {out}", shell=True, quiet_on_success=False)
    return Image(out)
        

def build_reps(src, asm, obj, function, gmon=None, run=True, *argc, **kwargs):
    if obj.endswith(".exe") and run:
        gprof = do_gprof(obj, gmon)
        call_graph = do_call_graph(obj, gmon, root=function)
    else:
        gprof = None
        call_graph = None

    pretty_loops = kwargs.pop("pretty_loops", False)
    if function:
        
        cfg_counts=do_cfg(obj,
                          output=f"{obj}-{function}-counts.png",
                          symbol=function,
                          jupyter=True,
                          spacing=kwargs.get('spacing',1),
                          inst_counts=True,
                          remove_assembly=False,
                          trim_addresses=False,
                          trim_comments=False,
                          pretty_loops=pretty_loops)
        cfg=do_cfg(obj,
                   output=f"{obj}-{function}-cfg.png",
                   symbol=function,
                   jupyter=True,
                   spacing=kwargs.get('spacing',1),
                   number_nodes=kwargs.get('number_nodes', False),
                   remove_assembly=kwargs.get('remove_assembly', False),
                   trim_addresses=kwargs.get('trim_addresses', True),
                   trim_comments=kwargs.get('trim_comments', False),
        pretty_loops=pretty_loops)
    else:
        cfg_counts = None
        cfg = None
        
    return RenderedCode(source=do_render_code(file=src, lang="c++", show=function),
                        asm=do_render_code(file=asm, lang="gas", demangle=kwargs.get("demangle", True), show=function),
                        cfg_counts=cfg_counts,
                        cfg=cfg,
                        gprof = gprof,
                        call_graph = call_graph)



def render_czoo(file, function, *argc, **kwargs):
    return dict(opt=build_reps(f"{file}_O4", function, *argc, **kwargs),
                unopt=build_reps(file, function, *argc, **kwargs))



def fiddle(fname, function=None, compile=True, code=None, opt=None, run=True, gprof=False, build_cmd=None, exe=None, **kwargs):
    if not run:
        gprof=False
    if gprof:
        run = True
    if not compile:
        run = False
    if opt is None:
        opt = ""

    
    if code is not None:
        updated = False

        if os.path.exists(fname):
            with open(fname, "r") as  f:
                old = f.read()
            if old != code: 
                updated = True
        else:
            updated = True

        if updated:
            if os.path.exists(fname):
                i = 0
                root, ext = os.path.splitext(fname)
                while True:
                    os.makedirs(".fiddle_backups", exist_ok=True)
                    p = f".fiddle_backups/{root}_{i:04}{ext}"
                    if not os.path.exists(p):
                        break

                    i += 1
                os.rename(fname, p)


            with open(fname, "w") as  f:
                f.write(code)
    
    
    base, _ =  os.path.splitext(fname)

    if exe is None:
        exe = f"{base}.exe"
        
    if compile:
        print("Compiling your code...")
        print("=====================================")
        if build_cmd is None:
            shell_cmd(f"rm -f build/{base}.s {base}.exe build/{base}.o")
            shell_cmd(f"make build/{base}.s {exe} C_OPTS='{opt}' {'GPROF=yes' if gprof else ''}", shell=True)
        else:
            shell_cmd(build_cmd, shell=True)
            
        if run:
            print("Running your code...")
            print("=====================================")
            shell_cmd(f"./{exe}")

    if function is not None or gprof:
        return build_reps(src=fname, asm=f"build/{base}.s", obj=exe, gprof=gprof, function=function, run=run, **kwargs)
    else:
        return None

def funcs(file, funcs, *argc, **kwargs):
    for f in funcs:

        display(HTML(f"<div style='text-align:center; font-weight: bold'><span>{f}</span></div>"))
        reps = build_reps(file, f, *argc, **kwargs)
        display(reps.source)    
        display(reps.asm)
        display(reps.cfg)
        display(reps.cfg_counts)


def side_by_side(function, *argc, **kwargs):
    data = render_czoo("czoo", function, *argc, **kwargs)
    display(HTML("<div style='text-align:center; font-weight: bold'><span>Source</span></div>"))
    display(data['opt'].source)    
    display(HTML("<div style='text-align:center; font-weight: bold'><span>Unoptimized</span></div>"))
    compare([data['unopt'].asm, data['unopt'].cfg])
    display(HTML("<div style='text-align:center; font-weight: bold'><span>Optimized</span></div>"))
    compare([data['opt'].asm, data['opt'].cfg])

def stacked(function, *argc, **kwargs):
    data = render_czoo("czoo", function, *argc, **kwargs)
    display(HTML("<div style='text-align:center; font-weight: bold'><span>Source</span></div>"))
    display(data['opt'].source)    
    display(HTML("<div style='text-align:center; font-weight: bold'><span>Unoptimized</span></div>"))
    display(data['unopt'].asm)
    display(data['unopt'].cfg)
    display(HTML("<div style='text-align:center; font-weight: bold'><span>Optimized</span></div>"))
    display(data['opt'].asm)
    display(data['opt'].cfg)

    
def czoo_compare2(function, *argc, **kwargs):
    file = "czoo"
    render_code(f"{file}.cpp", lang="c++", show=function),
    display(HTML("<div style='text-align:center; font-weight: bold'><span>Unoptimized</span></div>"))
    czoo2(function, optimize=False, *argc, **kwargs)
    display(HTML("<div style='text-align:center; font-weight: bold'><span>Optimized</span></div>"))
    czoo2(function, optimize=True, *argc, **kwargs)


def show_png(file):
    return Image(file)

def login(username):
    global user
    user = username
    return  IFrame(f"https://cse142l-dev.wl.r.appspot.com/user/web-login?email={username}", width=500, height=400)

def token(token):
    cmd = f"cse142 login {user} --token {token}"
    print(subprocess.check_output(cmd, shell=True).decode())
    
def plot1(file=None, df=None, field="per_element"):
    if df is None:
        df = render_csv(file)
    std=df[field].std()
    mean=df[field].mean()
    max_v =df[field].max()
    min_v =df[field].min()
    fix, axs = plt.subplots(1,2)
    axs[0].set_ylabel("per_element")
    axs[1].set_ylabel("Count")
    df[field].plot.line(ax=axs[0])
    df[field].plot.hist(ax=axs[1])
    axs[0].set_ybound(0,max_v*2)
    axs[0].set_xlabel("rep")
    axs[1].set_xlabel("per_element") 
    axs[1].set_xbound(1e-8,3e-8)
    plt.tight_layout()
    
    max_error = max(abs(max_v-mean),abs(min_v-mean))
    
    return pd.DataFrame.from_dict(dict(mean_per_rep=[df['ET'].mean()], mean=[mean], max=[max_v], min=[min_v], max_error_percent=[(max_error)/mean*100.0]))
    #f"Mean: {mean:1.2}; std: {std:1.2}; max: {max:1.2}; min: {min:1.2}; range: {max-min:1.2}; max-variation: {(max-min)/mean*100.0:1.2f}%"


from IPython.display import display, Markdown, Latex, Code, HTML
import re

def my_render(c):
    r = c._repr_html_()
    if r != None:
        return r
    else:
        return f'<img src="data:image/png;base64,{c._repr_png_()}">'
compare_style = """
            <style>
        .side-by-side {
            display: flex;
            align-items: stretch;
        }
        .side-by-side-pane {
            margin-right:1em;
            border-right-style: solid;
            border-right-color: black;
            border-right-width: 1px;
            flex: 1;
        }
        .side-by-side-pane .heading{
            font-size: 1.5;
            font-weight: bold;
            text-align:center;
            border-bottom-style: dotted;
            border-bottom-width: 1px;
            border-bottom-color: gray;
            margin-left: 1em;
            margin-right: 1em;

        }
        </style>
"""
def heading(s):
    display(HTML(f"""
    {compare_style}
        <div class="side-by-side"> """ +
                 
                 f"<div class='side-by-side-pane'><div class='heading'>{s}</div></div>" +


                 """
        </div>
    """))

    
def compare(content, headings=None):
    if headings is None:
        headings = [""] * len(content)
        
    display(HTML(f"""
    {compare_style}
        <div class="side-by-side"> """ +
                 
                 "".join([f"<div class='side-by-side-pane'><div class='heading'>{headings[i]}</div><div>{my_render(c)}</div></div>" for (i,c) in enumerate(content)]) +


                 """
        </div>
    """))

def render_code(*argc, **kwargs):
    display(do_render_code(*argc, **kwargs))

def do_render_code(file, lang="c++", show=None, line_numbers=True, trim_ends=False, demangle=None):

    if demangle is None:
        if lang == "gas":
            demangle = True
        else:
            demangle = False

    with open(file) as f:
        if demangle:
            lines = subprocess.check_output('c++filt', stdin=f).decode().split("\n")
        else:
            lines = open(file).read().split("\n")
        
    start_line = 0
    end_line = len(lines)

    if isinstance(show, str):
        if lang == "c++":
            show = (f"[\s\*]{re.escape(show)}\s*\(", "^\}")
        elif lang == "gas":
            show = (f"^{re.escape(show)}:\s*", ".cfi_endproc")
        else:
            raise Exception("Don't know how to find functions in {lang}")

    if show is not None and len(show) == 2:
        if isinstance(show[0], str): # regexes
            started = False
            for n, l in enumerate(lines):
                if not started:
                    if re.search(show[0], l):
                        start_line = n
                        started = True
                        
                else:
                    if re.search(show[1], l):
                        end_line = n + 1
                        break
        else:
            start_line, end_line = show
    elif show is not None:
        raise ValueError(f"{show} is not a good range for displaying code")
    
    if trim_ends:
        start_line += 1
        end_line -= 1
            
    comments = {"json": "//",
                "c++": "//",
                "gas": ";",
                "python" : "#"}
    
    src = f"{comments.get(lang, '//')} {file}:{start_line+1}-{end_line} ({end_line-start_line} lines)\n"
    src += "\n".join(lines[start_line:end_line])

    return Code(src, language=lang)

import pandas as pd

def render_csv(file, columns = None, sort_by=None, average_by=None, skip=0):
    df = pd.read_csv(file, sep=",")
    df = df[skip:]
    if sort_by:
        df = df.sort_values(by=sort_by)
    if average_by:
        df = df.groupby(average_by).mean()
        df[average_by] = df.index
    if columns:
        df = df[columns]
        
    return df

def _(csv, key, row, column, average_by=None):
    df = render_csv(csv, average_by=average_by)
    return df.filter(like="row", axis=key)[column]
    
    
from contextlib import contextmanager

@contextmanager
def layout(subplots, columns=4):
    f = plt.figure()
    rows = int(math.ceil(subplots/columns))
    f.set_size_inches(4*columns, 4*rows)
    def next_fig():
        c = 0
        while True:
            c += 1
            assert c <= subplots, "Too many subplots.  Increase the number you passed to layout()"
#            f.set_size_inches(4*columns, 4*rows)
            yield f.add_subplot(rows, columns, c)

    try:
        yield f, next_fig()
    finally:
        f.figsize=[4*columns, 4*rows]
        plt.tight_layout()
        

import matplotlib.pyplot as plt
def plot2(file=None, df=None, field="per_element"):
    if df is None:
        df = render_csv(file)

    df['incremental_average'] = incremental_average(df['per_element'])
    df['incremental_error'] = (df['per_element'].mean() - df['incremental_average'])/df['per_element'].mean()*100
    df['rep_error'] = (df['per_element'] - df['incremental_average'])/df['per_element'].mean()*100
    print(f"===================================\nMean = {df['per_element'].mean()}\nTotal execution time = {df['ET'].sum()}")

    std = df[field].std()
    mean = df[field].mean()
    max_v = df[field].max()
    min_v = df[field].min()
    
    with layout(subplots=1) as (fig, sub):
        axs = next(sub)
        
        axs.set_ylabel("per_element runtime")
        axs.set_ybound(0, df['per_element'].max()*2)
        axs.set_xlabel("rep")
        axs.set_title("Per-element time for each rep")
        df["per_element"].plot.line(ax=axs)
        axs.plot([0,200], [df["per_element"].mean(),df["per_element"].mean()], label="mean")
        axs.legend()
        return 
        
        axs = next(sub)
        axs.set_ylabel("absolute error (%)")
        axs.set_ybound(0,df['rep_error'].max()*2)
        axs.set_xlabel("rep")
        axs.set_title("Error compared to mean across all reps")
        df["rep_error"].abs().plot.line(ax=axs)

        axs.set_ylabel("absolute error (%)")
        axs.set_ybound(0,df['rep_error'].max()*2)
        axs.set_xlabel("rep")
        axs.set_title("Error compared to mean across all reps")
        df["rep_error"].abs().plot.line(ax=axs)

        axs = next(sub)
      
        axs.set_ylabel("incremental_average")
        axs.set_ybound(0,df['incremental_average'].max()*2)
        axs.set_xlabel("N")
        axs.set_title("Average over the first N reps")
        df["incremental_average"].plot.line(ax=axs)

        axs = next(sub)
        axs.set_ylabel("absolute error (%)")
        axs.set_ybound(0,df['incremental_error'].max()*2)
        axs.set_xlabel("rep")
        df["incremental_error"].abs().plot.line(ax=axs)

        axs = next(sub)
        axs.set_ylabel("Count")
        axs.set_xlabel("per_element") 
        axs.set_xbound(1e-8,3e-8)
        df["per_element"].plot.hist(ax=axs)



def plotPE(file=None, what=None, df=None, columns=4, log=False, average=False, average_by=None, dot_size=5):
    if df is None:
        df = render_csv(file,average_by=average_by)

    with layout(subplots=len(what), columns=columns) as (fig, sub):
        for i, (x, y) in enumerate(what):
            d = df.copy()
            axs = next(sub)
            axs.set_ylabel(y)
            axs.set_xlabel(x)
            axs.set_title(y)
            if log:
                axs.set_xscale("log")
                axs.set_yscale("log")
            else:
                axs.set_ybound(0, d[y].max()*1.3)
                axs.set_xbound(0, d[x].max()*1.3)
                axs.set_autoscalex_on(False)
                axs.set_autoscaley_on(False)

                
            if average:
                d = d.groupby(x).mean()
                d[x] = d.index
                #axs.plot.errorbar(x, y, yerr=1, fmt="o")
                d.plot.scatter(y=y, x=x, ax=axs, s=dot_size)

            else:
                d.plot.scatter(y=y, x=x, ax=axs, s=dot_size)

def plotPEBar(file=None, df=None, what=None, columns=4, log=False, average=False, average_by=None, skip=0, height=1):
    if df is None:
        df = render_csv(file,average_by=average_by, skip=skip)
        
    rows = int(math.ceil(len(what)/columns))
    f = plt.figure(figsize=[4*columns, 4*rows*height], dpi = 100)
    
    for i, (x, y) in enumerate(what):
        d = df.copy()
        axs = f.add_subplot(rows, columns, i+ 1)
        axs.set_ylabel(y)
        #axs.set_xlabel(x)
        axs.set_title(y)
        if log:
            #axs.set_xscale("log")
            axs.set_yscale("log")
        else:
            axs.set_ybound(0, d[y].max()*1.4)
            #axs.set_xbound(0, d[x].max()*1.1)
            axs.set_autoscalex_on(False)
            axs.set_autoscaley_on(False)
            
        if average:
            d = d.groupby(x).mean()
            d[x] = d.index
            d.plot.bar(y=y, x=x, ax=axs)
        else:
            d.plot.bar(y=y, x=x, ax=axs)
        
        for i, v in enumerate(d[y]):
            axs.text(i, v, f"{float(v):3.2}", ha='center' )
            
    plt.tight_layout()

    

            
def incremental_average(d):
    return [d[0:i+1].mean() for i in range(len(d))]

#plot2("200_first_try.csv")

def IC_avg_and_combine(*argc):
    all = render_csv(argc[0])
    all = all[0:0]

    for f in argc:
        df = render_csv(f)
        t = df['function'][0]
        r = dict(function=df['function'][0],
                 IC=df['IC'].sum(),
                 CPI=df['CPI'].mean(),
                 CT=df['CT'].mean(),
                 ET=df['ET'].sum(),
                 cmdlineMHz=df['cmdlineMHz'].mean(),
                 realMHz=df['realMHz'].mean())
        all = all.append(r, ignore_index=True)

    return all
