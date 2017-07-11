/****************************************************************************
 * Copyright 2005 Evan Drumwright
 * This library is distributed under the terms of the Apache V2.0 
 * License (obtainable from http://www.apache.org/licenses/LICENSE-2.0).
 ****************************************************************************/

#ifndef _SPHERE_PRIMITIVE_H
#define _SPHERE_PRIMITIVE_H

#include <Moby/Primitive.h>

namespace Moby {

class BoxPrimitive;
class BoundingSphere;

/// Represents a sphere primitive for inertia properties, collision detection, and visualization
class SpherePrimitive : public Primitive
{
  public: 
    SpherePrimitive();
    SpherePrimitive(double radius);
    SpherePrimitive(const Ravelin::Pose3d& T);
    SpherePrimitive(double radius, unsigned n);
    SpherePrimitive(double radius, const Ravelin::Pose3d& T);
    SpherePrimitive(double radius, unsigned n, const Ravelin::Pose3d& T);
    void set_radius(double radius);
    void set_num_points(unsigned n);
    bool is_convex() const override { return true; }
    void load_from_xml(boost::shared_ptr<const XMLTree> node, std::map<std::string, BasePtr>& id_map) override;
    void save_to_xml(XMLTreePtr node, std::list<boost::shared_ptr<const Base> >& shared_objects) const override;
    void set_pose(const Ravelin::Pose3d& T) override;
    void get_vertices(boost::shared_ptr<const Ravelin::Pose3d> P, std::vector<Point3d>& vertices) const override;
    BVPtr get_BVH_root(CollisionGeometryPtr geom) override;
    double calc_dist_and_normal(const Point3d& point, std::vector<Ravelin::Vector3d>& normals) const override;
    double calc_signed_dist(boost::shared_ptr<const Primitive> p, Point3d& pthis, Point3d& pp) const override;
    boost::shared_ptr<const IndexedTriArray> get_mesh(boost::shared_ptr<const Ravelin::Pose3d> P) override;
    osg::Node* create_visualization() override;
    double calc_signed_dist(boost::shared_ptr<const SpherePrimitive> s, Point3d& pthis, Point3d& psph) const;
    Point3d get_supporting_point(const Ravelin::Vector3d& d) const override;
    double calc_signed_dist(const Point3d& p) const override;
    double get_bounding_radius() const override { return _radius; }
    double get_maximum_compliant_layer_depth() const override { return get_compliant_layer_depth(); }

    /// Gets the radius for this sphere
    double get_radius() const { return _radius; }

    /// Gets the number of points used to create the sphere for visualization / collision checking
    unsigned get_num_points() const { return _npoints; }

  private:
    virtual void calc_mass_properties();
    static double calc_f(const Ravelin::VectorNd&, void*);
    static void calc_gradient(const Ravelin::VectorNd&, void*, Ravelin::VectorNd&);
    static void calc_hessian(const Ravelin::VectorNd&, void*, Ravelin::MatrixNd&);

    /// The bounding volumes for the sphere
    std::map<CollisionGeometryPtr, boost::shared_ptr<BoundingSphere> > _bsphs; 

    /// Radius of the sphere primitive
    double _radius;

    /// Number of points used to create collision geometry
    unsigned _npoints;
}; // end class

} // end namespace

#endif
