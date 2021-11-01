#pragma once

#include "ACollisionShape.h"

#include <vob/aoe/common/physic/Utils.h>

#include <vob/aoe/core/visitor/Utils.h>
#include <vob/aoe/core/type/Clone.h>
#include <vob/aoe/core/type/Primitive.h>

#include <bullet/BulletCollision/CollisionShapes/btCollisionShape.h>
#include <bullet/BulletCollision/CollisionShapes/btCompoundShape.h>

#include <vector>


namespace vob::aoe::common
{
    class CompoundShape final
        : public ACollisionShape
    {
        class Child
        {
        public:
            mat4 m_localMatrix{ 1.0f };
            type::Cloneable<ACollisionShape> m_collisionShape;

            explicit Child(type::Cloner<> const& a_cloner)
                : m_collisionShape{ a_cloner }
            {}

            template <typename VisitorType>
            void accept(VisitorType& a_visitor)
            {
                vec3 position;
                a_visitor.visit(vis::makeNameValuePair("LocalPosition", position));
                vec3 rotation;
                a_visitor.visit(vis::makeNameValuePair("LocalRotation", rotation));

                m_localMatrix = mat4{ 1.0f };
                m_localMatrix = glm::translate(m_localMatrix, position);
                m_localMatrix *= mat4{ quat{ rotation } };
                a_visitor.visit(vis::nvp("Shape", m_collisionShape));
            }

            template <typename VisitorType>
            void accept(VisitorType& a_visitor) const
            {
                static_assert(false && "TODO");
            }
        };

        struct ChildFactory
        {
            type::Cloner<> const& m_cloner;

            Child operator()() const
            {
                return Child{ m_cloner };
            }

        };

    public:
        // Constructors
        explicit CompoundShape(type::Cloner<> const& a_cloner)
            : m_cloner{ a_cloner }
        {}
        CompoundShape(CompoundShape const& a_other)
            : m_cloner{ a_other.m_cloner }
            , m_children{ a_other.m_children }
            , m_shape{
                a_other.m_shape.getDynamicAabbTree() != nullptr
                , static_cast<int>(a_other.m_children.size())
            }
        {
            addChildrenToShape();
        }

        // Operators
        CompoundShape& operator=(CompoundShape const& a_other)
        {
            m_shape = btCompoundShape{
                a_other.m_shape.getDynamicAabbTree() != nullptr
                , static_cast<int>(a_other.m_children.size())
            };
            m_children = a_other.m_children;
            addChildrenToShape();
        }

        // Methods
        btCollisionShape& getCollisionShape() override
        {
            return m_shape;
        }

        template <typename VisitorType>
        void accept(VisitorType& a_visitor)
        {
            auto enableDynamicAabbTree = true;
            a_visitor.visit(vis::nvp("EnableDynamicAabbTree", enableDynamicAabbTree));

            auto containerHolder = vis::cth(m_children, ChildFactory{ m_cloner });
            a_visitor.visit(vis::nvp("Children", containerHolder));

            // TODO: why does it not work? seems like copy operator is buggy
            // m_shape = btCompoundShape(enableDynamicAabbTree, static_cast<int>(m_children.size()));
            addChildrenToShape();
        }

        template <typename VisitorType>
        void accept(VisitorType& a_visitor) const
        {
            static_assert(false && "TODO");
        }

    private:
        type::Cloner<> const& m_cloner;
        std::pmr::vector<Child> m_children;
        btCompoundShape m_shape{};

        void addChildrenToShape()
        {
            for (auto& child : m_children)
            {
                m_shape.addChildShape(
                    toBtTransform(child.m_localMatrix)
                    , &child.m_collisionShape->getCollisionShape()
                );
            }
        }
    };
}

