#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


struct edge{
  int start, end, tb;
  double weight,ori_w;
};

struct vertex{
  int parent, id;
  double d;
};


struct edge ** edges;
struct edge ** edge_tb;
struct vertex * vertexs;
int * Q_map;
int Q_count;

struct edge** read_edges(char *filename, int *len){
  FILE *file = fopen(filename, "r");

  if(!file){
    fclose(file);
    return NULL;
  }

  int c = 0;
  struct edge ** readins;

  fscanf(file, "%d", len);
  readins = (struct edge **)malloc((*len)*(sizeof(struct edge *)));

  int readin = 0;
  while(fscanf(file, "%d", &readin) != EOF){
    int v = readin;
    fscanf(file, "%d", &readin);
    int e = readin;

    readins[c]= (struct edge*)malloc((e + 2)*(sizeof(struct edge)));
    readins[c][0].start = v; // number of vertex
    readins[c][0].end = e; // number of edge
    readins[c][0].weight = 0;

    for(int i=1;i<=e; i++){
      fscanf(file, "%d %d %lf",
        &(readins[c][i].start),
        &(readins[c][i].end),
        &(readins[c][i].weight));
    }
    fscanf(file, "%d %d", &(readins[c][e+1].start), &(readins[c][e+1].end)); // source and dist.
    c++;
  }

  fclose(file);
  return readins;
}


struct edge** read_edges_1graph(char *filename, int *len){
  FILE *file = fopen(filename, "r");

  if(!file){
    fclose(file);
    return NULL;
  }

  int c = 1;
  struct edge ** readins;

  // fscanf(file, "%d", len);
  *len = 1;
  readins = (struct edge **)malloc((*len)*(sizeof(struct edge *)));

  int read_state = 1;
  int current_node = 0;
  int node_num = 0;
  int max_weight = 0;

  readins[0]= (struct edge*)malloc((100)*(sizeof(struct edge)));

  while(!feof(file)){
    read_state = fscanf(file, "%d %d %d %lf\n",
      &current_node,
      &(readins[0][c].start),
      &(readins[0][c].end),
      &(readins[0][c].weight)
    );

    node_num = readins[0][c].start > node_num ? readins[0][c].start : node_num;
    node_num = readins[0][c].end > node_num ? readins[0][c].end : node_num;
    max_weight = readins[0][c].weight > max_weight ? readins[0][c].weight : max_weight;

    readins[0][c].start--;
    readins[0][c].end--;
    // readins[0][c].weight--;

    c++;
    if(c > 100)
      printf("Number of edge(link) aer limit to 100! \n");
  }

  for (size_t i = 1; i < c; i++) {
    readins[0][i].weight /= max_weight + 1;
    readins[0][i].weight = 1 - readins[0][i].weight;
  }

  readins[0][0].start = node_num; // number of vertex
  readins[0][0].end = c - 1; // number of edge
  readins[0][0].weight = 0;

  readins[0][c].start = -1;
  readins[0][c].end = -1; // source and dist.
  fclose(file);
  return readins;
}


int extract_max(int vertex_n){
  double max_v = -1, max = -1;

  for (int i = 0; i < vertex_n; i++) {
    if(Q_map[i] && vertexs[i].d > max_v) {// if vertex_i in set_Q
      max =  i;
      max_v = vertexs[i].d;
    }
  }
  return max;
}


void relax(int u, int vertex_n){
  for (int i = 0; i < vertex_n; i++) {
    if(edge_tb[u][i].tb){
      if(vertexs[i].d < vertexs[u].d * edge_tb[u][i].weight){ //we are deal with probability some using mul instead add.
        vertexs[i].d = vertexs[u].d * edge_tb[u][i].weight;
        vertexs[i].parent = u;
      }
    }
  }
}


void dijk_var_init(int vertex_n, int edge_group, int root, int dist){
  Q_count =  edges[edge_group][0].start - 1;

  edge_tb = (struct edge**)malloc((vertex_n) * sizeof(struct edge *));
  for(int k = 0; k < vertex_n; k++) edge_tb[k] = (struct edge*)malloc((vertex_n) * sizeof(struct edge));
  for(int k = 0; k < vertex_n; k++) memset(edge_tb[k], 0, vertex_n * sizeof(struct edge));

  for (int i = 1; i <= edges[edge_group][0].end; i++) { //setup edge_tb for edge & weight direct access for relax().
    int x = edges[edge_group][i].start, y = edges[edge_group][i].end;
    if(edge_tb[x][y].tb &&  edges[edge_group][i].weight < edge_tb[x][y].weight)
    continue;
    edge_tb[x][y].tb = 1;
    edge_tb[x][y].weight = edges[edge_group][i].weight;
  }

  Q_map = (int *)malloc(vertex_n * sizeof(int));
  for (int i = 0; i < vertex_n; i++) Q_map[i] = 1; // all vertexs in Q at start,
  Q_map[root] = 0; //except root.

  vertexs = (struct vertex *)malloc(vertex_n * sizeof(struct vertex));
  for(int k = 0; k < vertex_n; k++) vertexs[k].id = k, vertexs[k].parent = -1, vertexs[k].d = 0;
  vertexs[root].d = 1; // using probability as weight so setting key to 0 at root.
}


int dijk(int vertex_n, int edge_group, int root, int dist){
  dijk_var_init(vertex_n, edge_group, root, dist);
  printf("Init. \n");
  relax(root, vertex_n);
  printf("relax. \n");

  while(Q_count > 0){
    int u = extract_max(vertex_n); // find the vertex in Q have max distance.
    if(u != -1) { // if u founed.
      Q_map[u] = 0, --Q_count;
      relax(u, vertex_n);
    }
    else
      break;
  }

  //print out result path and probability.
  int t= dist, count = -1;
  int record[256];
  double p =1;

  while(t!=-1){
    record[++count] = t;
    if(vertexs[t].parent != -1)
      p *= edge_tb[vertexs[t].parent][t].weight;
    t = vertexs[t].parent;
  }

  printf("Shortest Path:\n");
  while(count > 0){
    printf("%d -> %d\n", record[count] + 1, record[count-1] + 1);
    --count;
  }
  printf("%lf\n\n", p);


  free(Q_map); // cleanup for next round.
  for (int i = 0; i < vertex_n; i++) free(edge_tb[i]);
  free(edge_tb);
  free(vertexs);

  return 1;
}


int main(int argc, char *argv[]) {
  if(argc != 4){
		printf("Usage: ./routing <topo.txt> <source> <destination>\n");
		return -1;
	}

  int len = 0;
  edges = read_edges_1graph(argv[1], &len);
  printf("Loaded. \n");

  for (size_t i = 0; i <= edges[0][0].end; i++) {
    printf("%d %d %lf\n", edges[0][i].start, edges[0][i].end, edges[0][i].weight);
  }

  dijk(
    edges[0][0].start,
    0,
    atoi(argv[2]),
    atoi(argv[3])
  );
  return 0;
}
