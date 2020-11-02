#pragma once

#include <memory>

#include <LevelAssets.h>
#include <IGameEntity.h>
#include <LevelContainer.h>

namespace ReGlacier
{
    template <typename T>
    concept IsContext = requires (T t) {
        t.Assets;
        t.Container;
        t.Container.get();
    };

    class GameEntityFactory
    {
    public:
        template <typename T, typename Context>
        static std::unique_ptr<T> Create(const std::string& name, Context* context)
            requires (IsContext<Context> && std::is_base_of_v<IGameEntity, T> &&
                      std::is_constructible_v<T, const std::string&, LevelContainer*, LevelAssets*>)
        {
            if (name.empty() || !context || !context->Container)
                return nullptr;

            return std::make_unique<T>(name, context->Container.get(), &context->Assets);
        }
    };
}