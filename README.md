# GraphResolver

`graph_resolver` registers modules by interface type, then constructs them in dependency order.
Each module implementation is registered from its plugin's `RegisterPlugin` function:

```cpp
extern "C" MODULE_API void RegisterPlugin(GetIGraphResolverFn get_resolver)
{
    auto* resolver = get_resolver();
    graph_resolver::AddModule<IMyInterface, MyModule,
        graph_core::TypeList<IModuleRequiredByConstructor>>(
            resolver,
            [](graph_resolver::IGraphResolver* graph) {
                return new MyModule(graph_resolver::GetModule<IModuleRequiredByConstructor>(graph));
            });
}
```

The third example demonstrates a two-phase dependency: its window module is constructed
independently, then receives the render module in its post-construction callback. Declare
constructor dependencies in `DEPS` and post-construction dependencies in `POSTDEPS`.
Both lists participate in graph validation and ordering.

When included as a subdirectory, examples and tests are disabled by default so only the
library is added to the parent project's build. Enable `GRAPH_RESOLVER_BUILD_EXAMPLES`
or `GRAPH_RESOLVER_BUILD_TESTS` to build them; both default to enabled when this
repository is configured directly.
