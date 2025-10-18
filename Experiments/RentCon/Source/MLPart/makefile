SRCS = ABKCommon ClusteredHGraph Combi Ctainers FilledHier FMPart GenHier Geoms HGraph MLPart PartEvals Partitioners Partitioning PartLegality Placement SmallPart Stats

all:
	@for src in $(SRCS); do\
		echo $$src;\
		cd $$src;\
		./make-gnu; cd ..;\
	done
clean:
	@\rm -rf libs/*
	@for src in $(SRCS); do\
		echo $$src;\
		cd $$src; make clean; cd ..;\
	done
