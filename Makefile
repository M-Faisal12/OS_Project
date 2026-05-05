all: main
main:
	g++ src/dispatcher.cpp -o dispatcher
	g++ src/ingester.cpp -o ingester
	g++ src/processor.cpp -o processor
	g++ src/reporter.cpp -o reporter
clean:
	rm -f dispatcher 
	rm -f ingester
	rm -f processor
	rm -f reporter
	rm -f pipe1
# 	rm -f logs/ingester.log
# 	rm -f logs/processor.log
# 	rm -f logs/reporter.log
	