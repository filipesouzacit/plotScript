#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Wed Mar 31 23:10:11 2021

@author: filipe
"""
import sys
import glob
import matplotlib.pyplot as plt
import numpy as np
import os

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print ("Error - Incorrect input")
        print ("Expecting python createPlot.py [testId] ")
        sys.exit(0)

    testId = sys.argv[1]
      
#testId = '10' 
          
    files = glob.glob('../solution/'+testId+'/dataPlot_*')
        
    if not os.path.exists('../data/plot/'+testId):
        os.makedirs('../data/plot/'+testId)
    
    inst = set([i.split('.')[-2] for i in files])
    
    
    for i in inst:
        outFile = '../data/plot/'+ testId + '/plot_'+ i + '.png'   
        outFile_after100I = '../data/plot/'+ testId + '/plot_'+ i + '_After100I.png'   
        files = glob.glob('../solution/'+testId+'/dataPlot_*.'+i+'.*')

        results = []
        for f in files:
            with open(f, 'r') as reader:
                r = [int(v) for v in reader.read().split(';')[:-1]]
            results.append(r)
                
        # plot 1
        plt.figure()
        for r in results:
            plt.plot([x for x in range(len(r))],r)
        plt.title('Cost Vs Iteration')
        plt.xlabel('Iteration')
        plt.ylabel('Cost')
        plt.yscale("log")
        plt.savefig(outFile, bbox_inches='tight')
        plt.close()
        
        # plot 2
        plt.figure()
        for r in results:
            plt.plot([x+100 for x in range(len(r[100:]))],r[100:])
        plt.title('Cost Vs Iteration')
        plt.xlabel('Iteration')
        plt.ylabel('Cost')
        plt.yscale("log")
        plt.savefig(outFile_after100I, bbox_inches='tight')
        plt.close()
        
        
        