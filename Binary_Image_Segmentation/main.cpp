//
//  main.cpp
//  Binary_Image_Segmentation
//
//  Created by Pavan Krishna Nandimandalam on 2020-10-31.
//

#include <opencv2/opencv.hpp>
#include <vector>
#include <stack>
#include <string>
#include <fstream>
#include <iostream>


using namespace cv;
using namespace std;

Mat in_image;
int vertex;

int sourcex;
int sourcey;
int sinkx;
int sinky;
int source;
int sink;
int maxFlow;
int minW=100;

Vec3b sourcePixel;
Vec3b sinkPixel;
Vec3b white(255,255,255);
Vec3b black(0,0,0);
Vec3b fore;
Vec3b back;

vector<int> color;

vector<vector<pair<int,int> > > adjList;
vector<int> path;
vector<int> reach;

vector<int> visit;

void updateGraph(int weight){
    for(int i=0;i<path.size();i++){
        if(adjList[path[i]].empty()){
            continue;
        }
        for(vector<pair<int,int> > :: iterator u=adjList[path[i]].begin();u!=adjList[path[i]].end();u++){
            int v=u->first;
            if(path[i+1]==v){
                int w=u->second;
                w=w-weight;
                adjList[path[i]].erase(u);
                adjList[path[i]].insert(u,make_pair(v, w));
               
            }
        }
    }

}
int findMin(){
    int min=INT_MAX;
    for(int i=0;i<path.size();i++){
        if(adjList[path[i]].empty()){
            continue;
        }
        for(vector<pair<int,int> > :: iterator u=adjList[path[i]].begin();u!=adjList[path[i]].end();u++){
            int v=u->first;
            if(path[i+1]==v){
                int w=u->second;
                if(w<min && w!=0){
                    min=w;
                }
            }
        }
    }
    return min;
}


bool DFS(int s,int t){
    path.clear();
    
    visit.resize(vertex,0);
    int u;
    int v;
    int w;
    stack<int> stac;
    stac.push(s);
   
    visit[s]=1;
    path.push_back(s);
    while(!stac.empty()){
        if(visit[t]==1){
            break;
        }
        u=stac.top();
        stac.pop();
        if(visit[u]==0){
            path.push_back(u);
            visit[u]=1;
        }
        int count=0;
        for(vector<pair<int,int> > :: iterator i=adjList[u].begin();i!=adjList[u].end();i++){
            v=i->first;
            w=i->second;
            if(visit[v]==0 && w!=0){
                stac.push(v);
        if(w<minW && w!=0)
            minW=w;
                count++;
            }
        }
        if(count==0&&u!=sink)
            path.pop_back();
        
    }
    
    if(visit[t]==1){
    return true;
    }
    else
        return false;
}
void ReachableFromSource(int s){
    int temp[vertex];
    vector<int> color;
    reach.clear();
    memset(temp, 0, sizeof(temp));
    temp[s]=1;
    for(int i=1;i<vertex;i++){
        
        if(DFS(s,i)){
            temp[i] = 1;
        }
        else{
            temp[i]=0;
        }
    }
    
    //storing the vertex value reachable form source in color
    for(int i=0;i<vertex;i++){
    color.push_back(temp[i]);
    }
  
    
}

Vec3b findColor(int width,int height,int x,int y){

    Vec3b a=in_image.at<Vec3b>(0,0);
    Vec3b b=in_image.at<Vec3b>(0,0);
    int topx,topy;
    int leftx,lefty;
    int rightx,righty;
    int botx,boty;

    topx=x;
    topy=y-1;
    leftx=x-1;
    lefty=y;
    rightx=x+1;
    righty=y;
    botx=x;
    boty=y+1;

    Vec3b temp1;
    Vec3b temp2;
    Vec3b temp3;
    int count[]={0,0,0};
    if(topy>=0){
        temp1=in_image.at<Vec3b>(topy,topx);
        count[0]=1;
    }

    if(leftx>=0){
        if(in_image.at<Vec3b>(lefty,leftx)==temp1)
            count[0]=count[0]+1;
        else{
            temp2=in_image.at<Vec3b>(lefty,leftx);
            count[1]=1;
        }
            
    }

    if(boty<height){
        if(in_image.at<Vec3b>(boty,botx)==temp1)
            count[0]=count[0]+1;

        else if(in_image.at<Vec3b>(boty,botx)==temp2)
            count[1]=count[1]+1;
        else{
            temp3=in_image.at<Vec3b>(boty,botx);
            count[2]=1;
        }
            
    }

    if(rightx<width){
        if(in_image.at<Vec3b>(righty,rightx)==temp1)
            count[0]=count[0]+1;

        else if(in_image.at<Vec3b>(righty,rightx)==temp2)
            count[1]=count[1]+1;
        else if(in_image.at<Vec3b>(righty,rightx)==temp3)
            count[2]=count[2]+1;
        
    }
    
    int max=-1;
    int index = 0;
    for(int i=0;i<3;i++){
        if(count[i]>max){
            max=count[i];
            index=i;
        }
    }
    
    if(index==0)
        return temp1;

    else if(index==1)
        return temp2;

    else
        return temp3;
}


int main( int argc, char** argv )
{
    cout << argc;
    if(argc!=4){
        cout<<"Usage: ../seg input_image initialization_file output_mask"<<endl;
        return -1;
    }
    
     in_image = imread(argv[1]/*, CV_LOAD_IMAGE_COLOR*/);
   
    if(!in_image.data)
    {
        cout<<"Could not load input image!!!"<<endl;
        return -1;
    }

    if(in_image.channels()!=3){
        cout<<"Image does not have 3 channels!!! "<<in_image.depth()<<endl;
        return -1;
    }
    
    // the output image
    Mat out_image = in_image.clone();
    Mat final_image = in_image.clone();
    
    ifstream f(argv[2]);
    
    if(!f){
        cout<<"Could not load initial mask file!!!"<<endl;
        return -1;
    }
    
    int width = in_image.cols;
    int height = in_image.rows;
    vertex = width*height;
    visit.resize(vertex,0);
    
    int n;
    f>>n;
    
    // get the initil pixels
    for(int i=0;i<n;++i){
        int x, y, t;
        f>>x>>y>>t;
        
        if(x<0 || x>=width || y<0 || y>=height){
            cout<<"I valid pixel mask!"<<endl;
            return -1;
        }
        
        
        Vec3b pixel;
        pixel[0] = 0;
        pixel[1] = 0;
        pixel[2] = 0;
        
        if(t==1){
            pixel[2] = 255;
        if(sinkPixel[2]==0){//get 1st sink coordinated
            sinkPixel=pixel;
            sinkx=x;
            sinky=y;
        }
        } else {
            pixel[0] = 255;
        if(sourcePixel[0]==0){ //get 1st source coordinates
            sourcePixel=pixel;
            sourcex=x;
            sourcey=y;
        }
        }
        
        out_image.at<Vec3b>(y, x) = pixel;
    }

//start
    
    fore = findColor(width,height,sinkx,sinky); //get majority neighbour pixel as foreground
    back = findColor(width,height,sourcex,sourcey); //get majority neighbour pixel as background
    
    //differentiat foreground color from background color
    for(int y=0;y<height;y++){
        for(int x=0;x<width;x++){

            if(out_image.at<Vec3b>(y,x)==back || out_image.at<Vec3b>(y,x)==sourcePixel){
                color.push_back(0);
            }
            
            else{
                color.push_back(1);
            }
        }
    }
    
    // creating graph from pixels and graph consist entry for every pixel in image.
    int pos;
    int w1=10;
    int w2=1;

    adjList.resize(height*width);
    for(int y=0;y<in_image.rows;y++){

        for(int x=0;x<in_image.cols;x++){
            
            if(y==sinky && x==sinkx){
                continue;
            }
            
            //left
            if(x-1>=0){
                pos=y*width+x;
                if(color.at(pos)==color.at(pos-1)){
                    adjList[pos].push_back(make_pair(pos-1,w1));
                }
                else{
                    adjList[pos].push_back(make_pair(pos-1,w2));
                }
            }

            //right
            if(x+1<out_image.cols){
                pos=y*width+x;
                if(color.at(pos)==color.at(pos+1)){
                    adjList[pos].push_back(make_pair(pos+1,w1));
                }
                else{
                    adjList[pos].push_back(make_pair(pos+1,w2));
                }
            }
            
            //top
            if(y-1>=0){
                pos=y*width+x;
                if(color.at(pos)==color.at((y-1)*width+x)){
                    adjList[pos].push_back(make_pair((y-1)*width+x,w1));
                }
                else{
                    adjList[pos].push_back(make_pair((y-1)*width+x,w2));
                }
            }
            
            //bottom
            if(y+1<out_image.rows){
                pos=y*width+x;
                if(color.at(pos)==color.at((y+1)*width+x)){
                    adjList[pos].push_back(make_pair((y+1)*width+x,w1));
                }
                else{
                    adjList[pos].push_back(make_pair((y-1)*width+x,w2));
                }
            }
        }
    }
        
    sink = sinky*width+sinkx;
    source=sourcey*width+sourcex;
    
    //calling MinCut Code i wrote on xcode
    int i=0;
    color.clear();
    while(DFS(source,sink)){
        i++;
        maxFlow = maxFlow+minW;
        updateGraph(minW);
        if(i==100000)
            break;
            path.clear();
        
        }
    
    
    //reachable from source
    ReachableFromSource(source);
    
    
    for(int y=0;y<height;y++){
        for(int x=0;x<width;x++){
            pos=y*width+x;
            if(color[pos]==0){
                final_image.at<Vec3b>(y,x)=black;
            }
            
            else{
                final_image.at<Vec3b>(y,x)=white;
            }
        }
    }
    // write it on disk
    imwrite( argv[3], out_image);
    
    // also display them both
    
    namedWindow( "Original image", WINDOW_AUTOSIZE );
    namedWindow( "Show Marked Pixels", WINDOW_AUTOSIZE );
    imshow( "Original image", in_image );
    imshow( "Show Marked Pixels", final_image );

    waitKey(0);
    return 0;
}
