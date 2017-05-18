
# coding: utf-8

# In[1]:

from bokeh.io import output_notebook, show

from bokeh.plotting import figure

import numpy as np

import pandas as pd

import serial
import time
# setup bokeh to output to the jupyter notebook
output_notebook()


# In[2]:

# This set's up the serial connection for the ECE544 Project2 Uartlite connection
print("setting up port")
ser = serial.Serial('COM4', 115200, timeout=1)
print(ser)


# In[3]:


df =pd.DataFrame()


# In[4]:

def update(df):
    data_str = ser.readline()
    temp = data_str.decode()
    df = df.append(dict([[y.strip() for y in x.split(':')] for x in temp.split(',')]), ignore_index=True)
    return df


# In[25]:

timeout = 60 #[secs]
timeout_start = time.time()
while time.time() < timeout_start + timeout:
    df=update(df)


# In[26]:

df.ActualRpm  = df.ActualRpm.astype(float)
df.DesiredRpm = df.DesiredRpm.astype(float)
# Example Line Plot in Bokeh
p = figure(plot_width=800, plot_height=400, title='Motor Control Plot')
p.line(df.index, df.ActualRpm, legend="Actual Rpm", line_color="blue")
p.line(df.index,df.DesiredRpm, legend="Desired Rpm", line_color="red")
p.line(df.index,df.MotorOut,legend=" Motor Out", line_color="purple")
p.xaxis.axis_label = "Time Elapsed(s)"
p.yaxis.axis_label = "RPM"
p.legend.location = "bottom_left"
p.legend.border_line_width = 2
p.legend.border_line_color = "black"
p.legend.border_line_alpha = 0.3
show(p)


# In[27]:

p2 = figure(plot_width=800, plot_height=400, title='Error & KP')
p2.line(df.index,df.KP, legend="KP",line_color="green")
p2.line(df.index,df.Error, legend="Error", line_color="red")
p2.xaxis.axis_label = "Time Elapsed(s)"
p2.yaxis.axis_label = "Value"
p2.legend.location = "bottom_left"
p2.legend.border_line_width = 2
p2.legend.border_line_color = "black"
p2.legend.border_line_alpha = 0.3
show(p2)


# In[ ]:

pd.to_csv("motorcontrolData.csv")

