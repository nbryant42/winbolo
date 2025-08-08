#Ryan2 2.4.3


OPTIONS = -I/usr/include/glib-1.2/ -I/usr/include/gtk-1.2/ -I/usr/lib/glib/include -Wall

ryan.so: Ryan.o setgui.o settings.o qksort.o issort.o actionqueue.o scout.o pointtank.o heap.o travelto.o refuel.o pickup.o attackpill.o attacktank.o laypill.o getman.o update.o gettrees.o repair.o baseguard.o
	 	gcc Ryan.o setgui.o settings.o qksort.o issort.o actionqueue.o scout.o pointtank.o heap.o travelto.o refuel.o pickup.o attackpill.o attacktank.o laypill.o getman.o update.o gettrees.o repair.o baseguard.o -o ryan2.so $(OPTIONS) -shared
	 	#To automatically install the brain, change the following line
	 	cp ryan2.so ../../brains -f

Ryan.o: Ryan.c
			gcc -c Ryan.c $(OPTIONS)

setgui.o: setgui.c
			gcc -c setgui.c $(OPTIONS)
			
settings.o: settings.c
			gcc -c settings.c $(OPTIONS)
			
qksort.o: qksort.c
			gcc -c qksort.c $(OPTIONS)
			
issort.o: issort.c
			gcc -c issort.c $(OPTIONS)

actionqueue.o: actionqueue.c
			gcc -c actionqueue.c $(OPTIONS)

scout.o: scout.c
			gcc -c scout.c $(OPTIONS)

pointtank.o: pointtank.c
			gcc -c pointtank.c $(OPTIONS)

heap.o: heap.c
			gcc -c heap.c $(OPTIONS)

travelto.o: travelto.c
			gcc -c travelto.c $(OPTIONS)

refuel.o: refuel.c
			gcc -c refuel.c $(OPTIONS)

pickup.o: pickup.c
			gcc -c pickup.c $(OPTIONS)

attackpill.o: attackpill.c
			gcc -c attackpill.c $(OPTIONS)

attacktank.o: attacktank.c
			gcc -c attacktank.c $(OPTIONS)

laypill.o: laypill.c
			gcc -c laypill.c $(OPTIONS)

getman.o: getman.c
			gcc -c getman.c $(OPTIONS)

update.o: update.c
			gcc -c update.c $(OPTIONS)

gettrees.o: gettrees.c
			gcc -c gettrees.c $(OPTIONS)

repair.o: repair.c
			gcc -c repair.c $(OPTIONS)

baseguard.o: baseguard.c
			gcc -c baseguard.c $(OPTIONS)


