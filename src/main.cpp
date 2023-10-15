#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include <entt/entt.hpp>

// Components

struct RenderStage
{
  enum StageID
  {
    PollEvents,
    UpdateDeltaTime,
    UpdatePhysics,
    PreRender,
    DebugRenderPhysics,
    PostRender,
    PresentDisplay
  } stage;
  using Callback = void(*)(entt::registry& );
  Callback fn;
};

// Registry Resources

struct DeltaTime
{
  float seconds;
};

struct RenderStageDirty
{
};

// Constructors

entt::entity add_stage(entt::registry& reg, RenderStage::StageID stage_id, RenderStage::Callback fn)
{
  auto stage = reg.create();
  reg.emplace< RenderStage >(stage, stage_id, fn);

  // mark the stages dirty
  reg.ctx().insert_or_assign< RenderStageDirty >(RenderStageDirty{});

  return stage;
}

// Main

int main()
{
  sf::RenderWindow window(sf::VideoMode(800, 600), "SFML works!");
  window.setVerticalSyncEnabled(true);

  entt::registry registry;

  registry.ctx().emplace< sf::RenderWindow& >(window);
  registry.ctx().emplace< sf::RenderTarget& >(window);
  registry.ctx().emplace< b2World >(b2Vec2{0,0});
  registry.ctx().emplace< DeltaTime >();
  registry.ctx().emplace< sf::Clock >();

  add_stage(registry, RenderStage::PollEvents,
    [](entt::registry& reg)
    {
      sf::Event event;
      auto& window = reg.ctx().get< sf::RenderWindow& >();
      while(window.pollEvent(event))
      {
        switch(event.type)
        {
        case sf::Event::KeyReleased:
          if(event.key.code == sf::Keyboard::Escape)
          {
            window.close();
          }
          break;

        case sf::Event::Closed:
          window.close();
          break;

        default:
          break;
        }
      }
    });

  add_stage(registry, RenderStage::UpdateDeltaTime, [](entt::registry& reg)
    {
      auto& clock = reg.ctx().get< sf::Clock >();
      auto& dt = reg.ctx().get< DeltaTime >();
      dt.seconds = clock.restart().asSeconds();
    });

  add_stage(registry, RenderStage::UpdatePhysics,
    [](entt::registry& reg)
    {
      const auto& dt = reg.ctx().get< DeltaTime >();
      reg.ctx().get< b2World >().Step(dt.seconds, 6, 2);
    });

  add_stage(registry, RenderStage::PreRender,
    [](entt::registry& reg)
    {
      auto& window = reg.ctx().get< sf::RenderWindow& >();
      window.clear();
    });

  add_stage(registry, RenderStage::PresentDisplay,
    [](entt::registry& reg)
    {
      auto& window = reg.ctx().get< sf::RenderWindow& >();
      window.display();
    });

  while(window.isOpen())
  {
    if(registry.ctx().contains< RenderStageDirty >())
    {
      registry.sort< RenderStage >([](const auto& lhs, const auto& rhs)
      {
        return lhs.stage < rhs.stage;
      });
      registry.ctx().erase< RenderStageDirty >();
    }

    auto render_stages = registry.view< RenderStage >();
    for(auto&& [entity, stage] : render_stages.each())
    {
      stage.fn(registry);
    }
  }

  return 0;
}
