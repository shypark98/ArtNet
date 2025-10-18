SRCS = lef

all:
	@for src in $(SRCS); do\
		echo $$src;\
		cd $$src; make; cd ..;\
	done

debug:
	@for src in $(SRCS); do\
		echo $$src;\
		cd $$src; make debug; cd ..;\
	done

clean:
	@for src in $(SRCS); do\
		echo $$src;\
		cd $$src; make clean; cd ..;\
	done
