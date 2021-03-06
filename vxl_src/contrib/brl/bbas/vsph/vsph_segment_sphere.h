#ifndef vsph_segment_sphere_h_
#define vsph_segment_sphere_h_
#include "vsph_unit_sphere.h"
//:
// \file
#include <vcl_map.h>
#include <vbl/vbl_disjoint_sets.h>

class vsph_segment_sphere
{
 public:
  vsph_segment_sphere(vsph_unit_sphere const& usph, double c, int min_size,double sigma, bool dosmoothing  )
  : seg_valid_(false),usph_(usph),sigma_(sigma), c_(c), min_size_(min_size),dosmoothing_(dosmoothing),num_ccs_(0) {}
  // === accessors ==-
  double sigma() const {return sigma_;}
  double c() const {return c_;}
  int min_size() const {return min_size_;}
  int num_ccs() const {return num_ccs_;}
  const vcl_map<int,  vcl_vector<int> >& regions() const {return regions_;}
  // === process methods ===
  void set_data(vcl_vector<double> data){data_ = data;}
  void segment();
  bool extract_region_bounding_boxes();
  const vcl_map<int, vsph_sph_box_2d> region_boxes() const {return bboxes_;}
  vcl_vector<double> region_data() const;
  vcl_vector<vcl_vector<float> > region_color() const;

  //: function to compute mean of the pixels in a region using the oringal values of the spherical segment
  double region_mean(int id);
  //: function to compute median of the pixels in a region using the oringal values of the spherical segment
  double region_median(int id);

  vsph_unit_sphere & unit_sphere(){return usph_;}
    vcl_vector<double> data_;
 private:
  void smooth_data();

  bool seg_valid_;
  vsph_unit_sphere usph_;
  double sigma_;
  double c_;
  int min_size_;
  bool dosmoothing_;
  //: spherical "image" data

  vcl_vector<double> smooth_data_;
  int num_ccs_;
  //: segmented regions
  vbl_disjoint_sets ds_;
  //: region id  sph vert ids
  vcl_map<int,  vcl_vector<int> > regions_;
  //: region id  box
  vcl_map<int, vsph_sph_box_2d> bboxes_;
};

#endif
