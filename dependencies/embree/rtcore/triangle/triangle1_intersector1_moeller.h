// ======================================================================== //
// Copyright 2009-2012 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#ifndef __EMBREE_ACCEL_TRIANGLE1_INTERSECTOR_MOELLER_H__
#define __EMBREE_ACCEL_TRIANGLE1_INTERSECTOR_MOELLER_H__

#include "triangle1.h"
#include "../common/ray.h"
#include "../common/hit.h"

namespace embree
{
  /*! Intersector for a single ray with individual precomputed
   *  triangles. This intersector implements a modified version of the
   *  Moeller Trumbore intersector from the paper "Fast, Minimum
   *  Storage Ray-Triangle Intersection". In contrast to the paper we
   *  precalculate some factors and factor the calculations
   *  differently to allow precalculating the cross product e1 x
   *  e2. The resulting algorithm is similar to the fastest one of the
   *  paper "Optimizing Ray-Triangle Intersection via Automated
   *  Search". */
  struct Triangle1IntersectorMoellerTrumbore
  {
    typedef Triangle1 Triangle;

    /*! Intersect a ray with the triangle and updates the hit. */
    static __forceinline void intersect(const Ray& ray, Hit& hit, const Triangle1& tri, const Vec3fa* vertices, const std::vector<int>& forbidden)
    {
      STAT3(normal.trav_tris,1,1,1);

      /* calculate determinant */
      const Vec3f O = ray.org;
      const Vec3f D = ray.dir;
      const Vec3f C = tri.v0 - O;
      const Vec3f R = cross(D,C);
      const float det = dot(tri.Ng,D);
      const float absDet = abs(det);
      const float sgnDet = signmsk(det);

      /* perform edge tests */
      const float U = xorf(dot(R,tri.e2),sgnDet);
      if (unlikely(U < 0.0f)) return;
      const float V = xorf(dot(R,tri.e1),sgnDet);
      if (unlikely(V < 0.0f)) return;
      const float W = absDet-U-V;
      if (unlikely(W < 0.0f)) return;
      
      /* perform depth test */
      const float T = xorf(dot(tri.Ng,C),sgnDet);
      if (unlikely(absDet*float(hit.t) < T)) return;
      if (unlikely(T < absDet*float(ray.near))) return;
      if (unlikely(det == float(zero))) return;

      /* update hit information */
      const float rcpAbsDet = rcp(absDet);
      hit.u   = U * rcpAbsDet;
      hit.v   = V * rcpAbsDet;
      hit.t   = T * rcpAbsDet;
      hit.id0 = tri.e1.a;
      hit.id1 = tri.e2.a;
    }

    /*! Test if the ray is occluded by one of the triangles. */
    static __forceinline bool occluded(const Ray& ray, const Triangle1& tri, const Vec3fa* vertices = NULL)
    {
      STAT3(shadow.trav_tris,1,1,1);

      /* calculate determinant */
      const Vec3f O = Vec3f(ray.org);
      const Vec3f D = Vec3f(ray.dir);
      const Vec3f C = tri.v0 - O;
      const Vec3f R = cross(D,C);
      const float det = dot(tri.Ng,D);
      const float absDet = abs(det);
      const float sgnDet = signmsk(det);

      /* perform edge tests */
      const float U = xorf(dot(R,tri.e2),sgnDet);
      if (unlikely(U < 0.0f)) return false;
      const float V = xorf(dot(R,tri.e1),sgnDet);
      if (unlikely(V < 0.0f)) return false;
      const float W = absDet-U-V;
      if (unlikely(W < 0.0f)) return false;
      
      /* perform depth test */
      const float T = xorf(dot(tri.Ng,C),sgnDet);
      if (unlikely(absDet*float(ray.far) < T)) return false;
      if (unlikely(T < absDet*float(ray.near))) return false;
      if (unlikely(det == float(zero))) return false;
      return true;
    }
  };
}

#endif


