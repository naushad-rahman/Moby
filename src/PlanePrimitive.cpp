/****************************************************************************
 * Copyright 2014 Evan Drumwright
 * This library is distributed under the terms of the Apache V2.0
 * License (obtainable from http://www.apache.org/licenses/LICENSE-2.0).
 ****************************************************************************/

#ifdef USE_OSG
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Material>
#include <osg/LightModel>
#endif
#include <boost/algorithm/minmax_element.hpp>
#include <Moby/Constants.h>
#include <Moby/CompGeom.h>
#include <Ravelin/sorted_pair>
#include <Moby/XMLTree.h>
#include <Moby/TorusPrimitive.h>
#include <Moby/BoundingSphere.h>
#include <Moby/CollisionGeometry.h>
#include <Moby/SpherePrimitive.h>
#include <Moby/CylinderPrimitive.h>
#include <Moby/PolyhedralPrimitive.h>
#include <Moby/GJK.h>
#include <Moby/PlanePrimitive.h>

using namespace Ravelin;
using namespace Moby;
using boost::shared_ptr;
using std::map;
using std::vector;
using std::list;
using std::pair;
using boost::dynamic_pointer_cast;
using boost::const_pointer_cast;
using std::make_pair;

/// Initializes the heightmap primitive
PlanePrimitive::PlanePrimitive()
{
}

/// Initializes the heightmap primitive
PlanePrimitive::PlanePrimitive(const Ravelin::Pose3d& T) : Primitive(T)
{
}

/// Gets the mesh of the heightmap
shared_ptr<const IndexedTriArray> PlanePrimitive::get_mesh(shared_ptr<const Pose3d> P)
{
  // TODO: not sure whether implementation is necesary
  assert(false);
  return shared_ptr<const IndexedTriArray>();
}

/// Transforms the primitive
void PlanePrimitive::set_pose(const Pose3d& p)
{
  // convert p to a shared pointer
  shared_ptr<Pose3d> x(new Pose3d(p));

  // determine the transformation from the old pose to the new one
  Transform3d T = Pose3d::calc_relative_pose(_F, x);

  // go ahead and set the new transform
  Primitive::set_pose(p);
}

/// Gets the BVH root for the heightmap
BVPtr PlanePrimitive::get_BVH_root(CollisionGeometryPtr geom)
{
  const unsigned X = 0, Y = 1, Z = 2;
  const double SZ = 1e+2;

  // get the pointer to the geometry
  shared_ptr<OBB>& obb = _obbs[geom];

  // if the OBB hasn't been created, create it and initialize it
  if (!obb)
  {
    // create the OBB
    obb = shared_ptr<OBB>(new OBB);
    obb->geom = geom;

    // get the pose for the geometry
    shared_ptr<const Pose3d> P = get_pose(geom);

    // setup the rotation matrix for the OBB
    obb->R.set_identity();

    // setup the translation for the OBB
    obb->center.set_zero(P);
    obb->center[Y] = -SZ*0.5;

    // setup obb dimensions
    obb->l.pose = P;
    obb->l[X] = SZ;
    obb->l[Y] = SZ;
    obb->l[Z] = SZ;
  }

  return obb;
}

/// Gets the vertices of the heightmap that could intersect with a given bounding volume
void PlanePrimitive::get_vertices(BVPtr bv, shared_ptr<const Pose3d> P, vector<Point3d>& vertices) const
{
  assert(_poses.find(const_pointer_cast<Pose3d>(P)) != _poses.end());
  const unsigned X = 0, Z = 2;

  // clear the vector of vertices
  vertices.clear();

  // get the bounding volume
  OBBPtr obb = dynamic_pointer_cast<OBB>(bv);
  obb->get_vertices(std::back_inserter(vertices));
}

/// Gets the vertices of the heightmap
void PlanePrimitive::get_vertices(shared_ptr<const Pose3d> P, vector<Point3d>& vertices) const
{
  const unsigned X = 0, Y = 1, Z = 2;
  const double SZ = 1e+2;
  assert(_poses.find(const_pointer_cast<Pose3d>(P)) != _poses.end() || P == get_pose());

  // clear the vector of vertices
  vertices.clear();

  // create an OBB
  OBB obb;

  // setup the rotation matrix for the OBB
  obb.R.set_identity();

  // setup the translation for the OBB
  obb.center.set_zero(P);
  obb.center[Y] = -SZ*0.5;

  // setup obb dimensions
  obb.l.pose = P;
  obb.l[X] = SZ;
  obb.l[Y] = SZ;
  obb.l[Z] = SZ;

  // get the vertices
  obb.get_vertices(std::back_inserter(vertices));
}

/// Computes the height at a particular point
double PlanePrimitive::calc_height(const Point3d& p) const
{
  assert(_poses.find(const_pointer_cast<Pose3d>(p.pose)) != _poses.end());
  const unsigned X = 0, Y = 1, Z = 2;

  return p[Y];
}

/// Gets the supporting point for the plane (for GJK)
Point3d PlanePrimitive::get_supporting_point(const Vector3d& dir) const
{
  const unsigned X = 0, Y = 1, Z = 2;

  assert(_poses.find(const_pointer_cast<Pose3d>(dir.pose)) != _poses.end());

  // direction is in this primitive's frame
  // case study:
  // if dir is [0 1 0] then supporting point is [0 0 0]
  // if dir is [0 -1 0] then supporting point is [0 -inf 0]
  // if dir is [x y z], where y >= 0 then supporting point is [x 0 z]
  // "               ", where y < 0 then supporting point is [x -inf z]
  Point3d p(dir[X], 0.0, dir[Z], dir.pose);
  if (dir[Y] >= 0.0)
    p[Y] = 0.0;
  else
    p[Y] = -1.0;  // note: we have to do this b/c INF messes with GJK

  return p;
}

/// Computes the OSG visualization
osg::Node* PlanePrimitive::create_visualization()
{
  #ifdef USE_OSG
  const unsigned X = 0, Y = 1, Z = 2;

  // get the pose and compute transform from the global frame to it
  shared_ptr<const Pose3d> P = get_pose();
  Transform3d T = Pose3d::calc_relative_pose(P, GLOBAL);

  // create necessary OSG elements for visualization
  osg::Group* group = new osg::Group;

  // create the necessary osg mechanicals
  osg::Group* subgroup = new osg::Group;
  osg::Geode* geode = new osg::Geode;
  subgroup->addChild(geode);
  group->addChild(subgroup);

  // create a new material with random color
  const float RED = (float) rand() / RAND_MAX;
  const float GREEN = (float) rand() / RAND_MAX;
  const float BLUE = (float) rand() / RAND_MAX;
  osg::Material* mat = new osg::Material;
  mat->setColorMode(osg::Material::DIFFUSE);
  mat->setDiffuse(osg::Material::FRONT, osg::Vec4(1.0f, 1.0f, 1.0f, 0.0f));
  subgroup->getOrCreateStateSet()->setAttribute(mat);

  // create the plane
  osg::InfinitePlane* plane = new osg::InfinitePlane;
  plane->set(0.0, 1.0, 0.0, 0.0);
  geode->addDrawable(new osg::ShapeDrawable(plane));

  return group;
  #else
  return NULL;
  #endif
}

/// Computes the signed distance of the given point from this primitive
double PlanePrimitive::calc_signed_dist(const Point3d& p) const
{
  assert(_poses.find(const_pointer_cast<Pose3d>(p.pose)) != _poses.end());

  // compute the height
  return calc_height(p);
}

/// Gets the distance from a cylinder primitive
double PlanePrimitive::calc_signed_dist(shared_ptr<const CylinderPrimitive> pA, Point3d& pthis, Point3d& pcyl) const
{
  FILE_LOG(LOG_COLDET) << "PlanePrimitive::calc_signed_dist(.) entered" << std::endl;

  // Output Params
  // Set intitial value of distance to contact
  double d = std::numeric_limits<double>::infinity();

  // get the pose for the plane primitive
  boost::shared_ptr<const Ravelin::Pose3d> Pplane = pthis.pose;
  // get the pose for the cylinder
  boost::shared_ptr<const Ravelin::Pose3d> Pcyl = pcyl.pose;

  const double R = pA->get_radius();
  const double H = pA->get_height();
  const unsigned X = 0, Y = 1,Z = 2;

  // Cylinder to Plane frame Tranformation
  Ravelin::Transform3d pPc = Ravelin::Pose3d::calc_relative_pose(Pcyl,Pplane);

  // Cylinder axis (in plane frame) cN
  // Take Y column from Cylinder to Plane frame Tranformation
  Ravelin::Vector3d cN = Ravelin::Vector3d(
                           Ravelin::Matrix3d(pPc.q).get_column(Y),
                           Pplane);
  cN.normalize();

  // Plane Normal is Y axis in local frame
  Ravelin::Vector3d n(0,1,0,Pplane);

  // cylinder origin w.r.t. plane
  Point3d c0(pPc.x.data(),Pplane);

  double n_dot_cN = n.dot(cN);

  // Axial direction points toward plane parallel to axis of cylinder
  Ravelin::Vector3d axial_dir = cN;
  if(n_dot_cN > 0)
    axial_dir = -axial_dir;
  axial_dir.normalize();

  // if Cylinder is aligned with plane:
  // Return distance cylinder origin to
  // closest point on plane minus height
  if(fabs(n_dot_cN) > 1.0-1e-8){
    FILE_LOG(LOG_COLDET) << " -- Cylinder axis is perpendicular to Plane" << std::endl;

    // Measure from center of closest face to the plane
    Point3d x = (H/2.0)*axial_dir + c0;

    // Distance above plane
    d = x.dot(n);

    // Find point on plane that x is closest to
    pcyl =  Ravelin::Pose3d::transform_point(Moby::GLOBAL,x);
    Point3d pP = x - d*n;
    pthis =  Ravelin::Pose3d::transform_point(Moby::GLOBAL,pP);
  } else if(fabs(n_dot_cN) < 1e-8){
    FILE_LOG(LOG_COLDET) << " -- Cylinder axis is parallel to Plane"<< std::endl;

    // Measure from center of closest edge (on curved face) to the plane
    // Radial direction and Plane normal coincide in this case
    Point3d x = c0 - R*n + (H/2.0)*axial_dir;

    // Distance above plane
    d = x.dot(n);

    pcyl =  Ravelin::Pose3d::transform_point(Moby::GLOBAL,x);
    Point3d pP = x - d*n;
    pthis =  Ravelin::Pose3d::transform_point(Moby::GLOBAL,pP);
  } 
  else {
    FILE_LOG(LOG_COLDET) << " -- Cylinder edge is closest to plane"<< std::endl;

    //(axis_cylinder x (n_plane x axis_cylinder))
    // Radial direction points toward plane perpendicular to axis of cylinder
    Ravelin::Vector3d radial_dir =
        Ravelin::Vector3d::cross(
          axial_dir,
          Ravelin::Vector3d::cross(axial_dir,n)
        );
    radial_dir.normalize();
     
    //                                            (--(|)  
    // Through axis, toward plane "axial direction"^, ^on face, toward plane "radial direction"
    FILE_LOG(LOG_COLDET) << "axial_dir = " << axial_dir << std::endl;
    FILE_LOG(LOG_COLDET) << "p_axis = " << n << std::endl;
    FILE_LOG(LOG_COLDET) << "radial_dir = " << radial_dir << std::endl;

    // Measure from point to the plane
    Point3d x = c0 + (H/2.0)*axial_dir + R*radial_dir;
    FILE_LOG(LOG_COLDET) << "x_cyl (plane frame) = " << x << std::endl;
    
    // Distance above plane
    d = x.dot(n);

    Point3d pP = x - d*n;
    FILE_LOG(LOG_COLDET) << "x_plane (plane frame) = " << x << std::endl;

    pcyl =  Ravelin::Pose3d::transform_point(Moby::GLOBAL,x);
    pthis =  Ravelin::Pose3d::transform_point(Moby::GLOBAL,pP);
  }

  FILE_LOG(LOG_COLDET) << "Point cylinder (global frame): "<<  pcyl << std::endl;
  FILE_LOG(LOG_COLDET) << "Point plane (global frame): "<<  pthis << std::endl;
  FILE_LOG(LOG_COLDET) << "distance: "<<  d << std::endl;

  return d;
}

/// Gets the distance from a polyhedral primitive
double PlanePrimitive::calc_signed_dist(shared_ptr<const PolyhedralPrimitive> b, Point3d& pthis, Point3d& pb) const
{
  const unsigned Y = 1;

  assert(_poses.find(const_pointer_cast<Pose3d>(pthis.pose)) != _poses.end());

  // make sure that the primitive is convex
  assert(b->is_convex());

  // compute the transform from the primitive to the plane
  Transform3d T = Pose3d::calc_relative_pose(pb.pose, pthis.pose);

  // setup initial minimum distance
  double min_dist = std::numeric_limits<double>::max();

  // get the polyhedron vertices
  shared_ptr<PolyhedralPrimitive> bnc = const_pointer_cast<PolyhedralPrimitive>(b);
  vector<Point3d> verts;
  bnc->get_vertices(pb.pose, verts);

  // find which vertex is closest in the plane space
  for (unsigned i=0; i< verts.size(); i++)
  {
    // get the polyhedron vertex in the plane space
    Point3d p_vert = T.transform_point(verts[i]);

    // get the vertex height
    if (p_vert[Y] < min_dist)
    {
      min_dist = p_vert[Y];
      pb = verts[i];
      pthis = p_vert;
    }
  }

  // closest point on plane is just the closest point on the plane, projected
  // to the plane
  pthis[Y] = 0.0;

  return min_dist;
}

/// Gets the distance from a sphere primitive
double PlanePrimitive::calc_signed_dist(shared_ptr<const SpherePrimitive> s, Point3d& pthis, Point3d& ps) const
{
  const unsigned Y = 1;

  assert(_poses.find(const_pointer_cast<Pose3d>(pthis.pose)) != _poses.end());

  // compute the transform from the sphere to the plane
  Transform3d T = Pose3d::calc_relative_pose(ps.pose, pthis.pose);

  // transform the sphere center to the plane space
  Point3d ps_c(0.0, 0.0, 0.0, ps.pose);
  Point3d ps_c_this = T.transform_point(ps_c);

  // get the lowest point on the sphere (toward the heightmap)
  Vector3d vdir(0.0, -1.0*s->get_radius(), 0.0, pthis.pose);

  // get the lowest point on the sphere
  Point3d sphere_lowest = ps_c_this + vdir;

  // setup the point on the plane
  pthis = ps_c_this;
  pthis[Y] = 0.0;

  // get the height of the sphere center
  ps = T.inverse_transform_point(sphere_lowest);
  return sphere_lowest[Y];
}

/// Finds the signed distance between the heightmap and another primitive
double PlanePrimitive::calc_signed_dist(shared_ptr<const Primitive> p, Point3d& pthis, Point3d& pp) const
{
  const unsigned Y = 1;

  // look for cylinder
  shared_ptr<const CylinderPrimitive> cyl = dynamic_pointer_cast<const CylinderPrimitive>(p);
  if (cyl)
    return calc_signed_dist(cyl, pthis, pp);

  // look for sphere
  shared_ptr<const SpherePrimitive> sph = dynamic_pointer_cast<const SpherePrimitive>(p);
  if (sph)
    return calc_signed_dist(sph, pthis, pp);

  // look for polyhedron
  shared_ptr<const PolyhedralPrimitive> polyhedron = dynamic_pointer_cast<const PolyhedralPrimitive>(p);
  if (polyhedron)
    return calc_signed_dist(polyhedron, pthis, pp);

  // look for torus
  shared_ptr<const TorusPrimitive> torus = dynamic_pointer_cast<const TorusPrimitive>(p);
  if (torus)
  {
    shared_ptr<const PlanePrimitive> shared_plane = dynamic_pointer_cast<const PlanePrimitive>(shared_from_this());
    shared_ptr<const Primitive> shared_this = shared_plane;
    return torus->calc_signed_dist(shared_this, pp, pthis);
  }
 

/*
  // if the primitive is convex, can use GJK
  if (p->is_convex())
  {
    shared_ptr<const Pose3d> Pplane = pthis.pose;
    shared_ptr<const Pose3d> Pgeneric = pp.pose;
    shared_ptr<const Primitive> prim_this = dynamic_pointer_cast<const Primitive>(shared_from_this());
    return GJK::do_gjk(prim_this, p, Pplane, Pgeneric, pthis, pp);
  }
*/
  // get p as non-const
  shared_ptr<Primitive> pnc = const_pointer_cast<Primitive>(p);

  // still here? no specialization; get all vertices from other primitive
  vector<Point3d> verts;
  pnc->get_vertices(pp.pose, verts);
  double mindist = std::numeric_limits<double>::max();
  for (unsigned i=0; i< verts.size(); i++)
  {
     Point3d pt = Pose3d::transform_point(pthis.pose, verts[i]);
     const double HEIGHT = calc_height(pt);
     if (HEIGHT < mindist)
     {
       mindist = HEIGHT;
       pp = verts[i];
       pthis = pt;
       pthis[Y] = -(HEIGHT - pt[Y]);
     }
  }

  return mindist;
}

/// Finds the signed distance betwen the plane and a point
double PlanePrimitive::calc_dist_and_normal(const Point3d& p, std::vector<Vector3d>& normals) const
{
  // setup the normal
  normals.clear();
  normals.push_back(Vector3d());
  Vector3d& normal = normals.back();

  // compute the distance
  double d = calc_height(p);

  // setup the normal
  normal = Vector3d(0.0, 1.0, 0.0, p.pose);

  // compute the distance
  return d;
}

/// Implements Base::load_from_xml() for serialization
void PlanePrimitive::load_from_xml(shared_ptr<const XMLTree> node, std::map<std::string, BasePtr>& id_map)
{
  // verify that the node type is heightmap
  assert(strcasecmp(node->name.c_str(), "Plane") == 0);

  // load the parent data
  Primitive::load_from_xml(node, id_map);
}

/// Implements Base::save_to_xml() for serialization
void PlanePrimitive::save_to_xml(XMLTreePtr node, std::list<shared_ptr<const Base> >& shared_objects) const
{
  // save the parent data
  Primitive::save_to_xml(node, shared_objects);

  // (re)set the node name
  node->name = "Plane";
}


