#pragma once

#include <../3rdparty/openmesh/src/OpenMesh/Tools/Subdivider/Uniform/SubdividerT.hh>
#include <../3rdparty/openmesh/src/OpenMesh/Tools/Subdivider/Uniform/LongestEdgeT.hh>
#include <../3rdparty/openmesh/src/OpenMesh/Core/Utils/vector_cast.hh>
#include <../3rdparty/openmesh/src/OpenMesh/Core/Utils/Property.hh>
// -------------------- STL
#include <vector>
#include <queue>

#if defined(OM_CC_MIPS)
#  include <math.h>
#else

#  include <cmath>

#endif


//== NAMESPACE ================================================================

namespace OpenMesh { // BEGIN_NS_OPENMESH
    namespace Subdivider { // BEGIN_NS_DECIMATER
        namespace Uniform { // BEGIN_NS_UNIFORM

            /// PLATEAUデータを格納するためのOpenMeshのプロパティ型を定義します。
            typedef OpenMesh::FPropHandleT<std::optional<int>> GameMaterialIDPropT;
            typedef OpenMesh::VPropHandleT<std::optional<TVec2f>> UV4PropT;
//== CLASS DEFINITION =========================================================


/** %Uniform LongestEdgeT subdivision algorithm
 *
 * Very simple algorithm splitting all edges which are longer than given via
 * set_max_edge_length(). The split is always performed on the longest
 * edge in the mesh.
 */
            template<typename MeshType, typename RealType = float>



/// *****************************************************************************
/// このコードはOpenMeshのLongestEdgeTをコピペして、PLATEAUのために一部改変したコードです。
/// *****************************************************************************

            /// OpenMeshのLongestEdgeTをコピーして、PLATEAU向けにカスタマイズしたSubdivision機能です。
            /// 細分化にあたって、gameMaterialID等が保持されるようになっています。
            class LongestEdgeDividerPlateau : public SubdividerT<MeshType, RealType> {
            public:

                typedef RealType real_t;
                typedef MeshType mesh_t;
                typedef SubdividerT <mesh_t, real_t> parent_t;

                typedef std::vector<std::vector<real_t> > weights_t;
                typedef std::vector<real_t> weight_t;

                typedef std::pair<typename mesh_t::EdgeHandle, real_t> queueElement;

            private:
                GameMaterialIDPropT game_material_id_prop;
                UV4PropT uv4_prop;
            public:


                LongestEdgeDividerPlateau(GameMaterialIDPropT game_mat_prop, UV4PropT uv4_prop) :
                parent_t(), game_material_id_prop(game_mat_prop), uv4_prop(uv4_prop) {}


                LongestEdgeDividerPlateau(mesh_t& _m, GameMaterialIDPropT game_mat_prop, UV4PropT uv4_prop) :
                    parent_t(_m), game_material_id_prop(game_mat_prop), uv4_prop(uv4_prop) {}


                ~LongestEdgeDividerPlateau() {}


            public:


                const char* name() const { return "Longest Edge Split"; }

                void set_max_edge_length(double _value) {
                    max_edge_length_squared_ = _value * _value;
                }

            protected:


                bool prepare(mesh_t& _m) {
                    return true;
                }


                bool cleanup(mesh_t& _m) {
                    return true;
                }


                bool subdivide(MeshType& _m, size_t _n, const bool _update_points = true) {

                    // Sorted queue containing all edges sorted by their decreasing length
                    std::priority_queue<queueElement, std::vector<queueElement>, CompareLengthFunction<mesh_t, real_t> > queue;

                    // Build the initial queue
                    // First element should be longest edge
                    typename mesh_t::EdgeIter edgesEnd = _m.edges_end();
                    for (typename mesh_t::EdgeIter eit = _m.edges_begin(); eit != edgesEnd; ++eit) {
                        const typename MeshType::Point to = _m.point(_m.to_vertex_handle(_m.halfedge_handle(*eit, 0)));
                        const typename MeshType::Point from = _m.point(
                                _m.from_vertex_handle(_m.halfedge_handle(*eit, 0)));

                        real_t length = (to - from).sqrnorm();

                        // Only push the edges that need to be split
                        if (length > max_edge_length_squared_)
                            queue.push(queueElement(*eit, length));
                    }

                    bool stop = false;
                    while (!stop && !queue.empty()) {
                        queueElement a = queue.top();
                        queue.pop();

                        if (a.second < max_edge_length_squared_) {
                            stop = true;
                            break;
                        } else {
                            const typename MeshType::Point to = _m.point(
                                    _m.to_vertex_handle(_m.halfedge_handle(a.first, 0)));
                            const typename MeshType::Point from = _m.point(
                                    _m.from_vertex_handle(_m.halfedge_handle(a.first, 0)));
                            const typename MeshType::Point midpoint =
                                    static_cast<typename MeshType::Point::value_type>(0.5) * (to + from);

                            const typename MeshType::VertexHandle newVertex = _m.add_vertex(midpoint);

                            // 分割前のGameMaterialIDを記録します
                            int game_mat_id = -1;
                            for(int i=0; i<2; i++) {
                                auto halfedge = _m.halfedge_handle(a.first, i);
                                if(!_m.is_boundary(halfedge)) {
                                    auto face = _m.face_handle(halfedge);
                                    game_mat_id = _m.property(game_material_id_prop, face).value_or(-1);
                                    break;
                                }
                            }

                            // 分割前のUV4を記録します
                            std::optional<TVec2f> uv4;
                            for(int i=0; i<2; i++) {
                                auto halfedge = _m.halfedge_handle(a.first, i);
                                auto vert = _m.from_vertex_handle(halfedge);
                                auto val = _m.property(uv4_prop, vert);
                                if(val.has_value()) {
                                    uv4 = val;
                                    break;
                                }
                            }
                            if(!uv4.has_value()) uv4 = TVec2f(-999, -999);


                            // ここで分割します
                            _m.split(a.first, newVertex);

                            // 分割により増えた面について、GameMaterialIDを埋めます。
                            for(int i = _m.n_faces() - 1; i >= 0; --i) {
                                auto f_handle = _m.face_handle(i);
                                auto prop = _m.property(game_material_id_prop, f_handle);
                                if (prop.has_value()) break;
                                _m.property(game_material_id_prop, f_handle) = game_mat_id;
                            }

                            // 分割により増えた点について、UV4を埋めます。
                            for(int i = _m.n_vertices() - 1; i >= 0; --i) {
                                auto v_handle = _m.vertex_handle(i);
                                auto prop = _m.property(uv4_prop, v_handle);
                                if (prop.has_value()) break;
                                _m.property(uv4_prop, v_handle) = uv4;
                            }


                            for (typename MeshType::VertexOHalfedgeIter voh_it(_m,
                                                                               newVertex); voh_it.is_valid(); ++voh_it) {
                                typename MeshType::EdgeHandle eh = _m.edge_handle(*voh_it);
                                const typename MeshType::Point to = _m.point(_m.to_vertex_handle(*voh_it));
                                const typename MeshType::Point from = _m.point(_m.from_vertex_handle(*voh_it));
                                real_t length = (to - from).sqrnorm();

                                // Only push the edges that need to be split
                                if (length > max_edge_length_squared_)
                                    queue.push(queueElement(eh, length));

                            }
                        }
                    }

#if defined(_DEBUG) || defined(DEBUG)
                    // Now we have an consistent mesh!
                    assert(OpenMesh::Utils::MeshCheckerT<mesh_t>(_m).check());
#endif


                    return true;
                }


            private: // data
                real_t max_edge_length_squared_;

            };

        } // END_NS_UNIFORM
    } // END_NS_SUBDIVIDER
} // END_NS_OPENMESH

