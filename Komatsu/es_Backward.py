import numpy as np
import pandas as pd
import seaborn as sns
import scipy 
from scipy import stats
from scipy.stats import circmean, circstd
from data_helper import FetchData
from es_cycle_parser import ESCycleParsing as cycle
from es_kpi import PropelKPI 
import datetime
from datetime import datetime
import math


class BackwardDetection:
    """Backward Detection Class"""
    def __init__(self):
        pass


    def back_angle (df , CL):
        """add 0,1 column to find shovel's back position 
        find the mean and standard deviation of upper_body_position for shovel's front side
        find confidence interval for shovel's front side
        find the indexes at which showel swings back and returns forward
        :param: ES_Swg_Res_Cnt , ES_Sys_RHM_SAO_State, confidence level
        :return: dataframe, time of swinging back and, time of returning forward
        """
        df['upper_body_position']=df.ES_Swg_Res_Cnt *360/8192
        c0=df.ES_Sys_RHM_SAO_State ==9.0 
        c1=df.ES_Sys_RHM_SAO_State ==5.0  
        c2=df.ES_Sys_RHM_SAO_State ==6.0 
        #c3=df.ES_Sys_RHM_SAO_State ==30.0 
        c4=df.ES_Sys_RHM_SAO_State ==10.0 
        #c5=df.ES_Sys_RHM_SAO_State ==8.0 
        c6=df.ES_Sys_RHM_SAO_State ==7.0 
        c7=df.ES_Sys_RHM_SAO_State ==11.0
        #dd=df[c0|c1|c2|c3|c4|c5|c6|c7]
        #dd=df[c0|c1|c2|c4|c6|c7]
        dd=df[c0|c1|c2]
        mu = circmean (dd['upper_body_position'], 360, 0)
        std=circstd(dd['upper_body_position'], 360, 0)
        conf_int = stats.norm.interval(CL, loc=mu, scale=std)
        f=conf_int[0]
        l=conf_int[1]

        if l>360:
            l=l-360
        if f<0:
            f=360+f
            print (f)

        m1=min(f,l)
        m2=max(f,l)
        print (m1)
        print (m2)
        if (mu > m1) and (mu < m2):
            df['back_angle']=np.where(np.logical_or(df['upper_body_position']<m1,df['upper_body_position']>m2),1,0)
        else:
            df['back_angle']=np.where(np.logical_and(df['upper_body_position']<m2,df['upper_body_position']>m1),1,0)

        df['intoback']=pd.DataFrame(df.back_angle.transpose().diff())
        df1=df.copy()
        df1.reset_index(inplace=True, drop=False)
        goInside=df1.index[df1.intoback ==1]
        goOutside=df1.index[df1.intoback ==-1]

        return df, goInside, goOutside


    def checkForState(T):
        """ check for  dig cycles, predig cycles and propel cycle in data"""
        start30, end30 = cycle.get_start_end_times(T.ES_Sys_RHM_SAO_State, 30)
        propcount= len(start30)
        start5, end5 = cycle.get_start_end_times(T.ES_Sys_RHM_SAO_State, 5)
        start6, end6 = cycle.get_start_end_times(T.ES_Sys_RHM_SAO_State, 6)
        digcount= len(start5)+ len(start6)
        dig = len(start6)
        predig= len(start5)
        return  digcount , propcount , predig , dig
    

    def propeltime(f):
        """calculate actual propel duration when propel mode is TRUE and tracks RPM is greater than
        15% of max RPM for 4CAC salesforce model
        """
        parser = cycle()
        time_start, time_end = parser.get_start_end_times(f['ES_Op_Indn_Ppl_Status'], 1)
        propel_cycles = pd.DataFrame(list(zip(time_start, time_end)),
                                     columns=['PropelCycle_Start_Time', 'PropelCycle_End_Time'])
        propel_cycles.index.name = 'ID'
        total_propel_cycle_both = []
        for z in range(propel_cycles.shape[0]):
            result = f.loc[np.logical_and(f.index >= propel_cycles.iloc[z].PropelCycle_Start_Time,
                                            f.index <= propel_cycles.iloc[z].PropelCycle_End_Time)]
            single_propel_cycle_both = PropelKPI.actual_propel_time_bothtrack(result)
            total_propel_cycle_both.append(single_propel_cycle_both)

        return  round(sum(total_propel_cycle_both),2)   

    

    def Backward (df, CL):
        """
        when the shovel swings back, pickes up the cable and returns forward (start time, end time, and duration, \
        and prople time for every Backward process for thr shovels)
        :param: ES_Swg_Res_Cnt , ES_Sys_RHM_SAO_State, ES_Sys_RHM_SAO_State, ES_Op_Indn_Ppl_Status,ES_Ppl_Mtr_Rt_Spd_RPM,ES_Ppl_Mtr_Lt_Spd_RPM and, confidence level
        :return: ['starttime' , 'endtime' , 'duration' , 'propelTime'] of each Backward 
        """
        TW=[0,0]
        Backward=[]
        propcount=0
        digcount=0          
      
        Possiblity=BackwardDetection.back_angle(df,CL) 
        if len(Possiblity[1])==len(Possiblity[2]):

            for i in range(len(Possiblity[1])-1): 
                if (Possiblity[0]['back_angle'].iloc[0]==1) and (i==0):
                    TW=Possiblity[0].iloc[:Possiblity[2][0]]
                elif (Possiblity[0]['back_angle'].iloc[0]==1) and (i!=0) and (i!=len(Possiblity[1])-1):  
                    TW=Possiblity[0].iloc[ Possiblity[1][i] : Possiblity[2][i+1]]
                elif (Possiblity[0]['back_angle'].iloc[0]==0):
                    TW=Possiblity[0].iloc[ Possiblity[1][i] : Possiblity[2][i]]
                    
                else:
                    TW=Possiblity[0].iloc[ Possiblity[1][i] : ]
                
                status=BackwardDetection.checkForState(TW)
                if (((status[0]>=3)|(status[2]>=2)|(status[3]>=2)) & (status[1]>=2)):  
                    propel=BackwardDetection.propeltime(TW)
                    Backward.append([TW.index[0], TW.index[len(TW)-1], \
                                       (TW.index[len(TW)-1]-TW.index[0])/1000,propel]) 

                TW=TW[0:0]
                propcount=0
                digcount=0
                continue 

        else: 
            for i in range(len(Possiblity[1])-1): 
                if (Possiblity[0]['back_angle'].iloc[0]==1) and (i==0):
                    TW=Possiblity[0].iloc[:Possiblity[2][0]]

                elif (Possiblity[0]['back_angle'].iloc[0]==1) and (i!=0):
                    TW=Possiblity[0].iloc[ Possiblity[1][i] : Possiblity[2][i+1]]
                elif (Possiblity[0]['back_angle'].iloc[0]==0) and (i!=len(Possiblity[1])-1):
                    TW=Possiblity[0].iloc[ Possiblity[1][i] : Possiblity[2][i]]
                else:
                    TW=Possiblity[0].iloc[ Possiblity[1][i] : ]
                                            

                status=BackwardDetection.checkForState(TW)
                if (((status[0]>=3)|(status[2]>=2)|(status[3]>=2)) & (status[1]>=2)):  
                    propel=BackwardDetection.propeltime(TW)
                    Backward.append([TW.index[0], TW.index[len(TW)-1], \
                                       (TW.index[len(TW)-1]-TW.index[0])/1000,propel]) 

                TW=TW[0:0]
                propcount=0
                digcount=0
                continue 



        Backward = pd.DataFrame (Backward , columns=['starttime' , 'endtime' , 'duration' , 'propelTime'])
        for i in range(len(Backward)):
            Backward.starttime.iloc[i]=datetime.utcfromtimestamp(Backward.starttime.iloc[i]/1000).strftime('%Y/%m/%d %H:%M:%S')
            Backward.endtime.iloc[i]=datetime.utcfromtimestamp(Backward.endtime.iloc[i]/1000).strftime('%Y/%m/%d %H:%M:%S')
        return Backward










