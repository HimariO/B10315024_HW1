#include <stdio.h>
#include <stdlib.h>
#define MAX_NUM_LINK 50
#define MAX_NUM_NODE 50

struct link{
        int ends[2];
        double cost;
};

struct node{
        int id;
        int next;
        double cost;
};

struct link links[MAX_NUM_LINK];
struct node nodes[MAX_NUM_NODE];
int num_of_link = 0;
int num_of_node = 0;



void readTopo(const char* plaintext){
	FILE* input_file = fopen(plaintext, "r");
	num_of_link = 0;
	num_of_node = 0;

        if(!input_file){
		printf("Cannot read the topo file\n");
		return;
	}

	while(!feof(input_file)){
		int dd;
                int id1;
                int id2;
		double cost;
		fscanf(input_file, "%d %d %d %lf\n", &dd, &id1, &id2, &cost);

		if(id1 > num_of_node) num_of_node = id1;
		if(id2 > num_of_node) num_of_node = id2;


		if(num_of_link < MAX_NUM_LINK){
			links[num_of_link].ends[0] = id1;
			links[num_of_link].ends[1] = id2;
			links[num_of_link].cost = cost;

			num_of_link++;
		}

		nodes[id1].next = -1;
		nodes[id1].cost = 0.0;
		nodes[id2].next = -1;
		nodes[id2].cost = 0.0;

	}


        fclose(input_file);
}


void print_path(int* path, int path_len){
	for(int i = 0; i < path_len; i++){
		printf("%d\t", path[i]);
	}
	printf("\n");
}

int main(int argc, char *argv[]){
	int path[MAX_NUM_NODE];
	int path_len;
	int source, destination;

	if(argc != 3){
		printf("Usage: ./routing <source> <destination>\n");
		return -1;
	}

	source = atoi(argv[1]);
	destination = atoi(argv[2]);


	readTopo("topo.txt");

	/* insert your code here and also you are free to define any function */

	print_path(path, path_len);
	
}
