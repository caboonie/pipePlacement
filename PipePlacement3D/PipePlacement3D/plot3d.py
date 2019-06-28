import csv
from mpl_toolkits.mplot3d import *# Axes3d
import matplotlib.pyplot as plt

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

filename = 'testPipes.csv'
componentFilename = "components_"+filename

with open(componentFilename) as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=',')
    line_count = 0
    points = [[],[],[]]
    for row in csv_reader:
        if(line_count!=0):
            if(float(row[3])>0):
                points[0].append(float(row[0]))
                points[1].append(float(row[1]))
                points[2].append(float(row[2]))
            else:
                print(float(row[0]),float(row[1]),float(row[2]),float(row[3]))
        line_count+=1
    ax.scatter(points[0],points[1],points[2],c='r',marker='o')

    ax.set_xlabel('x axis')
    ax.set_ylabel('y axis')
    ax.set_zlabel('z axis')

colors = ["tab:gray", "lightsteelblue", "pink", "coral", "tab:olive", "mediumseagreen", "b"]
colorI = 0

with open(filename) as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=',')
    line_count = 0
    for row in csv_reader:
        if(line_count!=0):
            if(row[0]=="group"):
                colorI+=1
                continue
            X = [float(row[0]),float(row[3])]
            Y = [float(row[1]),float(row[4])]
            Z = [float(row[2]),float(row[5])]
            ax.plot(X, Y, Z, c=colors[colorI%len(colors)])
        line_count+=1
        

plt.show()
        


