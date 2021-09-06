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
    
    def getMeanStd(i):
        l = []
        ll = []
        for r in range(len(results)):
            if len(results[r])>i:
                l.append(results[r][i])
                ll.append(resultsLog[r][i])
        return np.mean(l), np.std(l), np.mean(ll), np.std(ll)
     
#testId = '10' 
          
    files = glob.glob('../solution/'+testId+'/dataPlot_*')
        
    if not os.path.exists('../data/plot/'+testId):
        os.makedirs('../data/plot/'+testId)
    
    inst = set([i.split('.')[-2] for i in files])
    
    
    for i in inst:
        outFile_log = '../data/plot/'+ testId + '/plot_'+ i + '_log.png'
        outFileE_log = '../data/plot/'+ testId + '/plot_'+ i + '_error_log.png'
        outFile = '../data/plot/'+ testId + '/plot_'+ i + '.png'
        outFileE = '../data/plot/'+ testId + '/plot_'+ i + '_error.png'
        
        files = glob.glob('../solution/'+testId+'/dataPlot_*.'+i+'.*')
        results = []
        resultsLog = []
        maxLen = 0
        for f in files:
            r = np.genfromtxt(f, delimiter=';')[:-1]
            results.append(r)
            resultsLog.append(np.log(r))
            if maxLen < len(r):
                maxLen = len(r)
        
        meanStd = [getMeanStd(i) for i in range(maxLen)]
        
        y = [j[0] for j in meanStd]
        yL = [j[2] for j in meanStd]
        x = [j for j in range(len(y))]
        yerr = [j[1] for j in meanStd]
        yerrL = [j[3] for j in meanStd]
    
        
    
        # plot 1    
        fig, ax = plt.subplots()
        ax.errorbar(x, y,
                    yerr=yerr,
                    fmt='-')
         
        ax.set_xlabel('Iteration')
        ax.set_ylabel('Cost')
        ax.set_title('Cost Vs Iteration')
        plt.savefig(outFileE, bbox_inches='tight')
        plt.close()
        # plot 2
        plt.figure()
        plt.plot(x,y)
        plt.title('Cost Vs Iteration')
        plt.xlabel('Iteration')
        plt.ylabel('Cost')
        plt.savefig(outFile, bbox_inches='tight')
        plt.close()
        
        # plot 3    
        fig, ax = plt.subplots()
        ax.errorbar(x, yL,
                    yerr=yerrL,
                    fmt='-')
         
        ax.set_xlabel('Iteration')
        ax.set_ylabel('Log(Cost)')
        ax.set_title('Cost Vs Iteration')
        plt.savefig(outFileE_log, bbox_inches='tight')
        plt.close()
        # plot 4
        plt.figure()
        plt.plot(x,yL)
        plt.title('Cost Vs Iteration')
        plt.xlabel('Iteration')
        plt.ylabel('Log(Cost)')
        plt.savefig(outFile_log, bbox_inches='tight')
        plt.close()
        
        