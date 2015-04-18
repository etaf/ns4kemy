#!/usr/bin/env python
# encoding: utf-8

from optparse import OptionParser
import os
import subprocess
import numpy as np
import matplotlib.pyplot as plt

def get_parameters():
    parser = OptionParser()
    parser.add_option("--result", type="string",dest="result_dir",default="results")
    #parser.add_option("--config", type="string",dest="conffile",default="config/func-eval.tcl")
    parser.add_option("-w","--whiskers",  type="string",dest="whiskers", default=os.path.join(cwd,"/home/lxa/F.3.23"))
    parser.add_option("--func", type="string", dest="evaluate_func", default = "onoff")
    (config, args) = parser.parse_args()
    return config

def func_evaluate(result_dir, nTCPsrc, nUDPsrc):
    """functional evaluate"""
    candidates = ['KEMY','RED']

    if  os.path.exists(result_dir):
        while True:
            ret = raw_input("Results already existed, remove and continue ? y/n\t")
            if ret == 'y':
                os.system("rm -rf "+result_dir)
                os.makedirs(result_dir)
                break
            elif ret == 'n':
                return
    else:
        os.makedirs(result_dir)

    evaluate(result_dir, "./config/func-eval.tcl", candidates, "Application/FTP", nTCPsrc, nUDPsrc)
    for candidate in candidates:
        awk_process(result_dir+"/"+candidate)

def light_eval(result_base):
    """light 5 TCP flows simulate 100s"""
    result_dir = os.path.join(result_base, 'light')
    func_evaluate(result_dir, 5, 0)

    graph_base_simtime(result_dir, "light: 5 TCP Flows")

def heavy_eval(resut_base):
    """heavy 50 TCP flows simulate 100s"""
    result_dir = os.path.join(result_base, 'heavy')
    func_evaluate(result_dir, 50, 0)
    graph_base_simtime(result_dir, "Heavy: 50 TCP Flows")
    #graph_box(result_dir, "Heavy: 50 TCP Flows")
    #graph_cdf(result_dir, "Heavy: 50 TCP Flows")

def mix_eavl():
    """mix 5 TCP + 2 UDP flows simulate 100s"""
    result_dir = os.path.join(result_base, 'mix')
    func_evaluate(result_dir, 5, 2 )
    graph_base_simtime(result_dir, "Mixture: 5 TCP + 2 UDP Flows")

def burst_eval():
    pass

def onoff_eval(result_base):
    candidates = ["KEMY", "CoDel", "PIE"]
    result_dir = os.path.join(result_base, 'onoff')
    do_eveal = True
    if  os.path.exists(result_dir):
        while True:
            ret = raw_input("Results already existed, remove and continue ? y/n\t")
            if ret == 'y':
                os.system("rm -rf "+result_dir)
                os.makedirs(result_dir)
                break
            elif ret == 'n':
                do_eveal = False
                break
    else:
        os.makedirs(result_dir)
    if do_eveal == True:
        for i in xrange(10):
            evaluate(result_dir, "./config/func-eval.tcl", candidates, "Application/OnOff", 8, 0 )
    for candidate in candidates:
        result_file = os.path.join(result_dir, candidate)
        subprocess.call(["awk -f ./awks/onoff_throughput.awk " + result_file+" >"  + result_file+".throughput" ], shell=True)
        subprocess.call(["awk -f ./awks/onoff_delay.awk " + result_file+" >"  + result_file+".delay" ], shell=True)

    graph_box(result_dir,candidates, "")
    #graph_cdf(result_dir, "Heavy: 50 TCP Flows")

def evaluate(result_dir, conffile, candidates, tcp_app, ntcpsrc, nudpsrc):
    """evaluate an AQM by run run-test.tcl """

    if tcp_app == "Application/OnOff":
        trace_type = '-onoff_out'
    else :
        trace_type = '-qtr'

    child_ps = []
    for candidate in candidates:
        tcl_args = ['./run-test.tcl', \
                                          conffile,\
                                          '-nTCPsrc', str(ntcpsrc),\
                                          '-tcp_app', tcp_app,\
                                          '-nUDPsrc', str(nudpsrc),\
                                          '-bw', '15',\
                                          '-delay', '150',\
                                          '-gw', candidate,\
                                          trace_type, result_dir+"/"+candidate, \
                                          #'-nam', candidate+'.nam',\
                                          ]
#        if onoff_out != "":
            #tcl_args = tcl_args + ['-onoff_out', os.path.join(onoff_out,"onoff-"+candidate)]
        child_ps.append(subprocess.Popen(tcl_args))
        print " ".join(tcl_args)

    for child_p in child_ps:
        child_p.wait()



def awk_process(result_file):
    """awk process and graph"""

    # throughput
    subprocess.call(["awk -f ./awks/inst_throughput.awk " + result_file+" >"  + result_file+".throughput" ], shell=True)
    # delay
    subprocess.call(["awk -f ./awks/insta_delay.awk " + result_file+" >"  + result_file+".delay" ], shell=True)
    # drop_rate
    #subprocess.call(["awk -f ./awks/drop_rate.awk " + result_file+" >"  + result_file+".drop_rate" ], shell=True)


def graph_base_simtime(result_dir, graph_title):
    """graph base simulation time"""
    #graph x:simulation time, y: metric
    #metrics = ['throughput', 'delay', 'drop_rate']
    metrics = ['throughput', 'delay']
    ylabels = {}
    ylabels['throughput'] = 'Throughput [Mbps]'
    ylabels['delay'] = 'Queueing Delay [msec]'
    ylabels['drop_rate'] = 'Drop Rate'
    for metric in metrics:
        plt.title(graph_title)
        plt.grid()
        plt.xlabel("Simulation Time [Sec]")
        plt.ylabel(ylabels[metric])

        for candidate in candidates:
            result_file = os.path.join(result_dir, candidate) +"." + metric
            data = np.loadtxt(result_file, unpack = True)
            line, =plt.plot(data[0],data[1], label = candidate)

        plt.legend()
        plt.savefig( os.path.join(result_dir,metric) +".svg", format="svg")
        plt.show()

def graph_box(result_dir, candidates, graph_title):
    metrics = ['throughput', 'delay']
    for metric in metrics:
        plt.title(graph_title)
        plt.grid(True)
        data = []
        for candidate in candidates:
            result_file = os.path.join(result_dir, candidate + "." + metric)
            tmp = np.loadtxt(result_file, unpack = True)
            data.append( tmp )
        boxs= plt.boxplot(data, showfliers = False)
        plt.xticks(range(1,len(candidates)+1), candidates)
        plt.savefig(os.path.join(result_dir, metric+"-box.svg"))
        plt.show()
def graph_cdf(result_dir, graph_title):
    metrics = ['delay','throughput']
    for metric in metrics:
        plt.title(graph_title)
        plt.grid(True)
        for candidate in candidates:
            result_file = os.path.join(result_dir, candidate + "." + metric)
            data = np.loadtxt(result_file, unpack = True)
            sorted_data = np.sort(data[1])
            yvals = np.arange(len(sorted_data)) / float(len(sorted_data))
            line, = plt.plot(sorted_data, yvals,label = candidate)
        plt.legend()
        plt.savefig(os.path.join(result_dir, metric+"-cdf.svg"))
        plt.show()







if __name__ == '__main__':
    cwd = os.getcwd()
    config = get_parameters()
    #candidates = ["KEMY", "RED","PIE","CoDel"]
    candidates = ["KEMY", "CoDel", "PIE"]
    #candidates = ["KEMY","RED"]
    result_base = os.path.join(cwd, config.result_dir)
    os.environ['WHISKERS'] = config.whiskers
    print config.whiskers

    evaluate_func = config.evaluate_func
    if evaluate_func == "light":
        light_eval()
    elif evaluate_func == "heavy":
        heavy_eval()
    elif evaluate_func == "mix":
        mix_eavl()
    elif evaluate_func == "onoff":
        onoff_eval(result_base)
