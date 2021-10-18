#pragma once

#include "ACollisionShape.h"

#include <memory>

#include <bullet/BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <bullet/BulletCollision/CollisionShapes/btCollisionShape.h>
#include <bullet/BulletCollision/CollisionShapes/btStridingMeshInterface.h>

#include <vob/aoe/common/render/model/StaticModel.h>


namespace vob::aoe::common
{
    class ModelShape final
        : public ACollisionShape
    {
        class StridingStaticMesh
            : public btStridingMeshInterface
        {
        public:
            StridingStaticMesh(std::shared_ptr<common::GraphicResourceHandle<common::StaticModel> const> a_model = nullptr)
                : m_model{ std::move(a_model) }
            {}

            void getLockedVertexIndexBase(
                unsigned char** vertexBase
                , int& numVertices
                , PHY_ScalarType& type
                , int& stride
                , unsigned char** indexBase
                , int& indexStride
                , int& numFaces
                , PHY_ScalarType& indicesType
                , int subpart
            ) override
            {
                assert(false);
            }

            void getLockedReadOnlyVertexIndexBase(
                unsigned char const** vertexBase
                , int& numVertices
                , PHY_ScalarType& type
                , int& stride
                , unsigned char const** indexBase
                , int& indexStride
                , int& numFaces
                , PHY_ScalarType& indicesType
                , int subpart
            ) const override
            {
                type = PHY_FLOAT;
                stride = sizeof(Vertex);
                indicesType = PHY_INTEGER;
                indexStride = sizeof(Triangle);

                if (m_model == nullptr)
                {
                    numVertices = 0;
                    *vertexBase = nullptr;
                    numFaces = 0;
                    *indexBase = nullptr;
                }

                assert(subpart < m_model->resource()->m_meshes.size());
                auto const& mesh = m_model->resource()->m_meshes[subpart];
                numVertices = static_cast<int>(mesh.getVertices().size());
                *vertexBase = reinterpret_cast<unsigned char const*>(&mesh.getVertices()[0]);
                numFaces = static_cast<int>(mesh.getTriangles().size());
                *indexBase = reinterpret_cast<unsigned char const*>(&mesh.getTriangles()[0]);
            }

            void unLockVertexBase(int subpart) override
            {
                assert(false);
            }

            void unLockReadOnlyVertexBase(int subpart) const override
            {
                // noop
            }

            int getNumSubParts() const override
            {
                if (m_model == nullptr)
                {
                    return 0;
                }
                return static_cast<int>(m_model->resource()->m_meshes.size());
            }

            void preallocateVertices(int numVertices) override
            {
                assert(false);
            }

            void preallocateIndices(int numIndices) override
            {
                assert(false);
            }

            std::shared_ptr<common::GraphicResourceHandle<common::StaticModel> const> m_model;
        };

    public:
        ModelShape()
            : m_shape{ &m_stridingMesh, true, false }
        {}

        ModelShape(ModelShape&& a_other) noexcept
            : m_stridingMesh{ a_other.m_stridingMesh }
            , m_shape{
                &m_stridingMesh
                , false
        }
        {}

        ModelShape(ModelShape const& a_other)
            : m_stridingMesh{ a_other.m_stridingMesh }
            , m_shape{
                &m_stridingMesh
                , false
            }
        {}

        virtual ~ModelShape() = default;

        ModelShape& operator=(ModelShape&& a_other) noexcept
        {
            m_stridingMesh.m_model = std::move(a_other.m_stridingMesh.m_model);
            return *this;
        }

        ModelShape& operator=(ModelShape const& a_other)
        {
            m_stridingMesh.m_model = a_other.m_stridingMesh.m_model;
            return *this;
        }

        void setModel(std::shared_ptr<common::GraphicResourceHandle<common::StaticModel> const> a_model)
        {
            m_stridingMesh.m_model = std::move(a_model);
        }

        btCollisionShape& getCollisionShape() override
        {
            return m_shape;
        }

        template <typename VisitorType, typename Self>
        static void accept(VisitorType& a_visitor, Self& a_this)
        {
            a_visitor.visit(vis::nvp("Model", a_this.m_stridingMesh.m_model));
        }

    private:
        StridingStaticMesh m_stridingMesh;
        btBvhTriangleMeshShape m_shape;
    };
}
