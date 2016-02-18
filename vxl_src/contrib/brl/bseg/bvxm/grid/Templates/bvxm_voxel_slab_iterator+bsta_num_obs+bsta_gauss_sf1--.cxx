#include <bvxm/grid/bvxm_voxel_slab_iterator.txx>
#include <bsta/bsta_attributes.h>
#include <bsta/bsta_gauss_sf1.h>
#include <bsta/io/bsta_io_attributes.h>
#include <bsta/io/bsta_io_gaussian_sphere.h>

typedef bsta_num_obs<bsta_gauss_sf1> gauss_type;
BVXM_VOXEL_SLAB_ITERATOR_INSTANTIATE(gauss_type);
BVXM_VOXEL_SLAB_ITERATOR_INSTANTIATE(bsta_gauss_sf1);