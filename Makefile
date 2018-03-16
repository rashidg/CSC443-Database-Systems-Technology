CC = gcc

all: disk_sort max_ave_followers

disk_sort: disk_sort.c
	$(CC) -o $@ $< 

max_ave_followers: max_ave_followers.c
	$(CC) -o $@ $< 

clean:
	rm ./disk_sort
	rm ./max_ave_followers