import csv
import turtle

xscale=11
yscale=11
biggestDiam = 0;
xoffset = 850
filename = 'testPipes.csv'
componentFilename = "components_"+filename
turtle.delay(.1)

with open(filename) as csv_file:
    csv_reader = csv.reader(csv_file, delimiter=',')
    line_count = 0
    for row in csv_reader:
        if(line_count!=0):
            #print(float(row[4]))
            biggestDiam = max(float(row[4]),biggestDiam)
        line_count+=1

#input("wait")

        
with open(filename) as csv_file:
    pipeScale = 1/biggestDiam
    csv_reader = csv.reader(csv_file, delimiter=',')
    line_count = 0
    for row in csv_reader:
        #input("wait")
        if(line_count!=0):
            #a = input("")
            turtle.penup()
            turtle.goto(float(row[0])*xscale-xoffset,float(row[1])*yscale)
            turtle.pendown()
            turtle.width(max(float(row[6])*pipeScale,.5))
            turtle.goto(float(row[3])*xscale-xoffset,float(row[4])*yscale)
        line_count+=1

for header in [-5,5]:
    turtle.penup()
    turtle.goto(float(-1000)*xscale-xoffset,header*yscale)
    turtle.pendown()
    turtle.color('green')
    turtle.goto(float(1000)*xscale-xoffset,header*yscale)


for bulkx in [42.5,75,107.5]:
    turtle.penup()
    turtle.goto(float(bulkx)*xscale-xoffset,float(-12)*yscale)
    turtle.pendown()
    turtle.color('red')
    turtle.goto(float(bulkx)*xscale-xoffset,float(12)*yscale)
    
with open(componentFilename) as csv_file:
    biggestComp = 0
    csv_reader = csv.reader(csv_file, delimiter=',')
    line_count = 0
    for row in csv_reader:
        if(line_count!=0):
            biggestComp = max(biggestComp, float(row[2]))
        line_count+=1
        
print(biggestComp)
with open(componentFilename) as csv_file:
    compScale = 2/biggestComp
    csv_reader = csv.reader(csv_file, delimiter=',')
    line_count = 0
    turtle.shape('circle')
    turtle.resizemode("user")
    for row in csv_reader:
        if(line_count!=0):
            turtle.penup()
            turtle.goto(float(row[0])*xscale-xoffset,float(row[1])*yscale)
            turtle.pendown()
            turtle.color("blue")
            turtle.shapesize(max((float(row[2])*compScale)**.5,.05))
            turtle.stamp()
        line_count+=1



