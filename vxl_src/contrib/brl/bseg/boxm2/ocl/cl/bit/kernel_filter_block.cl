//:
// \file
// \brief  Methods to apply a kernel/filter to a block of a bit scene
// \author Isabel Restrepo
// \date 4/13/12

//requires cell_minx
inline int calc_blkI(float cell_minx, float cell_miny, float cell_minz, int4 dims)
{
  return convert_int(cell_minz + (cell_miny + cell_minx*dims.y)*dims.z);
}

inline void get_max_inner_outer(int* MAX_INNER_CELLS, int* MAX_CELLS, int root_level)
{
  //USE rootlevel to determine MAX_INNER and MAX_CELLS
  if(root_level == 1) 
    *MAX_INNER_CELLS=1, *MAX_CELLS=9; 
  else if (root_level == 2) 
    *MAX_INNER_CELLS=9, *MAX_CELLS=73; 
  else if (root_level == 3) 
    *MAX_INNER_CELLS=73, *MAX_CELLS=585; 
}


//:Apply a filter, passed as coefficients, for a block
// Notes: 
// 1.Keep the size of the kernel manageble, as you could run into resources issues
// 2.As of now this runs at the lowest level of the octrees 
__kernel void kernel_filter_block(  __constant RenderSceneInfo * linfo,
                                    __global   uchar16         * trees,
                                    __global   float           * data_in,                //could be alpha or any other float quantity
                                    __global   float           * filter_response,        //holds the response to kernel
                                    __global   float4          * filter,                 //holds position and coefficient
                                    __constant unsigned        * filter_size,              //number of coefficients                                  
                                    __constant uchar           * lookup,
                                    __constant float           * centerX,                //center of current cell
                                    __constant float           * centerY, 
                                    __constant float           * centerZ,
                                    __local    uchar16         * all_local_tree,
                                    __local    uchar16         * all_neighbor_tree
                                    )
{
  //make sure local_tree points to the right one in shared memory
  uchar llid = (uchar)(get_local_id(0) + (get_local_id(1) + get_local_id(2)*get_local_size(1))*get_local_size(0)); 
  __local uchar16* local_tree    = &all_local_tree[llid]; 
  __local uchar16* neighbor_tree = &all_neighbor_tree[llid]; 
  
  
  //global id should be the same as treeIndex
  unsigned gidX = get_global_id(0);
  unsigned gidY = get_global_id(1); 
  unsigned gidZ = get_global_id(2); 

  //The resolution of the kernels is fixed at the highest res cell

  float side_len = 1.0f/(float) (1<<3); 
  
  //tree Index is global id 
  unsigned treeIndex = calc_blkI(gidX, gidY, gidZ, linfo->dims); 
  if(gidX < linfo->dims.x && gidY < linfo->dims.y && gidZ <linfo->dims.z) 
  {
    int MAX_INNER_CELLS, MAX_CELLS;
    get_max_inner_outer(&MAX_INNER_CELLS, &MAX_CELLS, linfo->root_level); 
    
    //get current tree information
    (*local_tree)    = trees[treeIndex]; 
    
    //loop over all leaves on the local tree
    for(int i = 0; i < MAX_CELLS; ++i) 
    {             
      if( is_leaf(local_tree, i) ) 
      {
         
        //get properties of current leaf cell
        int currDepth = get_depth(i);
        float response = 0.0; 
        
//        if(currDepth != linfo->root_level)
//          continue;
        
        float side_len = 1.0f/(float) (1<<currDepth);
        float4 localCenter = (float4) (centerX[i], centerY[i], centerZ[i], 0.0f);  //local center within block
        float4 cellCenter = (float4) ( (float) gidX + centerX[i],                  //abs center within block
                                      (float) gidY + centerY[i], 
                                      (float) gidZ + centerZ[i], 
                                      0.0f );  
        
        //iterate through the filter coefficients and positions     
        for( uint ci = 0; ci < filter_size[0]; ci++ )
        {
            
          //get neighbor location, and corresponding tree (may be current tree, may not) -- it may be more efficient to collect all neighbors in a linked  list...
          float4 nbCenter = (float4) (cellCenter.x+(side_len*filter[ci].x), cellCenter.y+(side_len*filter[ci].y), cellCenter.z+(side_len*filter[ci].z), 0.0f);
          
          //check if neighbor is out of bounds
          bool in_bounds = (nbCenter.x > 0) && (nbCenter.x < linfo->dims.x);
          in_bounds = in_bounds && (nbCenter.y > 0) && (nbCenter.y < linfo->dims.y);
          in_bounds = in_bounds && (nbCenter.z > 0) && (nbCenter.z < linfo->dims.z);
          
          if (!in_bounds){
            response = 0.0;
            break;
          }
           
          int blkI = calc_blkI( floor(nbCenter.x), 
                                floor(nbCenter.y), 
                                floor(nbCenter.z), linfo->dims);  
          
          (*neighbor_tree) = trees[blkI];
                   
          //get neighbor local center, traverse to it
          float4 locCenter = nbCenter - floor(nbCenter); 
          int neighborBitIndex = (int) traverse_to(neighbor_tree, locCenter, currDepth);
          
          if( is_leaf(neighbor_tree, neighborBitIndex) )
          {
   
            //get data index
            int idx = data_index_relative( neighbor_tree, neighborBitIndex, lookup) + data_index_root(neighbor_tree); 
           
            //grab alpha, calculate probability
            float alpha = data_in[idx];
            float prob = 1.0f - exp(-alpha * side_len * linfo->block_len); 
            
            response += prob * filter[ci].w;
          }

        } 
        
        //store response
        int currIdx = data_index_relative(local_tree, i, lookup) + data_index_root(local_tree); 
        filter_response[currIdx] = response; 
                      
      }
    }
  }
}


