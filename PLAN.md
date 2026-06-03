这份技术文档旨在向负责实现本框架的开发人员完整交付 **Godot-FairyGUI (GDExtension)** 的核心设计思想、底层架构蓝图及具体实现规范。
本框架的设计原则是：**“渲染求极致，事件顺天命，动效自立棍”**。我们不采取低效的“引擎外套壳”方案，而是深度融合 Godot 底层管线，打造一套原生、高性能、数据驱动的 UI 解决方案。
# 📖 项目概述与核心目标
### 1. 背景与痛点
传统的第三方 UI 框架引入 Godot 时，往往采用独立于引擎层级之外的“黑盒渲染空间”方案，导致输入事件流与引擎 3D/2D 世界严重割裂。本文档选择主动拥抱 Godot 的 Control 体系，让 FairyGUI 对象成为原生 UI 节点，再通过自绘、图集、虚拟列表与对象池控制性能成本。
### 2. 核心目标
本框架基于 **Godot 4.5** 与 **GDExtension (C++)** 编写底层 Runtime、资产导入器与编辑器辅助工具。框架以 FairyGUI 编辑器导出的 **.bytes 包**为核心输入，对齐 Unity FairyGUI Runtime 的主要 API 语义，但输出一套符合 Godot 哲学的新 Runtime。
运行时命名风格与 Unity 版本保持一致：**GObject、GComponent、GImage、GTextField、GLoader、GList、UIPackage、GTween**。其中 **GObject 继承 Godot Control**，GComponent、GImage、GTextField、GLoader 等均继承自 GObject，既对齐 Unity Runtime 的对象模型，也充分吃到 Godot 原生布局、裁剪、输入、信号与编辑器能力。公开 API 方法命名优先对齐 Unity FairyGUI Runtime，以维护 FairyGUI 多引擎版本的统一心智。
通过**原生 Control 节点树**、**组件自绘（重写 _draw）**、**接管原生输入事件冒泡**、以及**独立的 C++ 动效内核**，在高保真还原 FairyGUI 编辑器功能的同时，实现工业级的高性能。
本 workspace 的目标不是交付某个 MVP 或停留在某个阶段验收点，而是持续完成一套**完整、工程可用、覆盖全面、符合 Godot 哲学且 API 优雅的 FairyGUI on Godot**。后续 Milestone 仅用于组织研发顺序、降低验证风险与形成阶段性检查点，不构成停止条件。
# 🏗️ 第一部分：核心系统架构设计
本框架由四个核心层级组成，开发时需严格遵循以下职责划分：
```
[ FairyGUI 编辑器 ] ──导出──> .bytes + 图集
                                   │
                                   ▼ (UIPackage.add_package / 可选 EditorImportPlugin)
[ 1. 资产管线层 ] ───────────> UIPackage 运行时内存结构
                                   │
                                   ▼ (运行时根据配置动态组装)
[ 2. 核心节点层 ] ───────────> GObject (继承自 Control)
                                   │
                             ├── GComponent / GImage / GTextField / GLoader
                             └── 3. 事件驱动层 (原生 Control 冒泡 / PixelHitTestData)

```
## 1. 资产管线层 (Asset Pipeline)
资产管线以 Runtime 直接加载为核心，可按需编写扩展自 **EditorImportPlugin** 的编辑器辅助插件。
 * **目标版本：** 工程对齐 Godot 4.5，当前以本地 Godot 4.5.1 与 `godot-cpp` 官方 `godot-4.5-stable` tag 作为 ABI 验证基线。不得使用会生成 4.6 ABI 的 `godot-cpp` 分支来构建本仓库，避免 GDExtension 在 Godot 4.5 中加载失败。后续 EditorImportPlugin 行为与编辑器预览能力也以该 4.5 基线优先验证。
 * **构建系统：** GDExtension 工程使用 **CMake** 组织，方便 IDE 集成、跨平台构建与后续 CI。
 * **仓库结构：** `godot-cpp/` 作为 submodule 目录存在；核心源码放在 `src/`；Godot 专用紧凑 demo 放在 `example/`；编辑器、导出、代码生成等辅助脚本放在 `tools/`；后续可按需要补充 `cmake/`、`include/`、`thirdparty/` 等常见工程目录。
 * **Runtime 优先：** 对齐 Unity FairyGUI Runtime，框架核心不是重 Editor Import，而是运行时直接面向 `.bytes` 包描述。`UIPackage.add_package(...)` 采用 Godot 风格 snake_case 暴露，同时保持 Unity Runtime 的语义。
 * **数据解包：** 引擎侧核心输入与 Unity Runtime 一致，聚焦 FairyGUI 导出的 **.bytes** 二进制包。FairyGUI 编辑器工程中的 XML 可作为设计期参考与测试素材来源，但不作为 Runtime 的首要输入格式。
 * **数据承载：** `.bytes` 解析后进入运行时内存结构（UIPackage、PackageItem、AtlasSprite、PixelHitTestData、raw component data 等），不强制生成可见中间文件。必要时可使用 Godot Resource 包装包描述，服务编辑器预览、Inspector 引用与缓存，但不得破坏运行时直接加载 `.bytes` 的能力。
 * **资源加载分离：** 包描述解析与纹理、声音、字体等资源加载解耦。PackageItem 记录文件名、图集 sprite rect、九宫格、PixelHitTestData 等元数据；实际 Texture2D、AudioStream、Font 等由同步/异步加载回调、Godot ResourceLoader、FileAccess、PCK 或热更新资源系统按需提供。
 * **加载入口设计：** `add_package(path)` 负责 Godot 路径语义：`res://` 与已挂载 PCK 内资源优先走 ResourceLoader/FileAccess 读取；`user://` 与外部热更新目录优先走 FileAccess 读取 `.bytes`，再由同目录或自定义 loader 提供图集、字体、声音。`add_package_bytes(bytes, asset_name_prefix)` 负责原始字节输入；`add_package_bytes_with_loader(bytes, asset_name_prefix, loader)` 负责原始包字节加自定义资源来源；`add_package_with_loader(asset_name_prefix, loader)` 负责完全自定义资源来源。loader 采用 Godot Callable 形式，接收资源路径并返回 PackedByteArray 或 Texture2D。可选 ResourceFormatLoader 可让 `.bytes` 被 Godot `load()` 识别，但不作为 Runtime 必需依赖。
 * **图集池化：** 运行时在加载图集纹理后，根据 `.bytes` 中的 atlas sprite 信息构造或缓存共享底层纹理的 AtlasTexture/NTexture 包装，确保相同纹理资产拥有稳定、可复用的句柄。
 * **Editor Import 定位：** EditorImportPlugin 若实现，应定位为“Godot 编辑器识别、预览、依赖追踪与便捷配置”工具，而不是必须的资产转换关卡。它不应批量产生多余文件；reimport 应自然刷新缓存与预览状态。
## 2. 核心节点与渲染层 (Runtime Node & Rendering)
框架采用**“GObject 即 Control”**的拓扑结构，让 FairyGUI 对象直接进入 Godot 原生 UI 树。
 * **GObject (基础节点)：** 继承自 Godot 的 Control，承载位置、尺寸、可见性、Gear、Relation、命中测试、事件信号与基础属性映射。
 * **GComponent / GImage / GTextField / GLoader：** 均继承自 GObject。GComponent 作为容器节点组织子 GObject；GImage 与普通 GTextField 通过重写 _draw 自绘；GLoader 负责统一资源加载。
 * **渲染去冗余（以 GImage 为例）：** 普通的静态图片组件**禁止**在内部 add_child(TextureRect)。虽然 GImage 自身是 Control，但不再额外包一层 TextureRect，避免无意义节点嵌套。
   * *实现规范：* GImage 直接重写 **_draw()**，调用 CanvasItem 绘制命令：
   ```cpp
   // GDExtension C++ 极致渲染示例
   void GImage::_draw() {
       if (atlas_texture.is_null()) return;
   
       if (has_scale9_grid) {
           // 调用底层九宫格绘制，由 GPU 级高效完成拉伸，完美自动合批
           draw_texture_rect_region_style(
               atlas_texture->get_atlas(), 
               Rect2(Vector2(), get_size()), 
               atlas_texture->get_region(), 
               scale9_margins
           );
       } else {
           draw_texture_rect(atlas_texture, Rect2(Vector2(), get_size()), false);
       }
   }
   
   ```
   * *合批原理：* 由于底层连续渲染的 GImage 持有的是相同大图集的 AtlasTexture 指针，Godot 底层的 RenderingServer 会在渲染管线中自动将其合并为单次 Draw Call。
## 3. 事件驱动层 (Native Event Loop)
框架放弃自研内部事件树，全面融入 Godot 的原生 Control 输入链。
 * **输入映射：** GObject 的 touchable 属性直接映射为 Godot 的 mouse_filter（STOP / PASS / IGNORE）。Godot 负责目标拾取、事件冒泡、父子遮挡关系与宿主节点和其他 Control、2D、3D 对象之间的输入顺序。
 * **像素级穿透判定：** 对齐 Unity Runtime 和 FairyGUI 导出数据，统一使用 **PixelHitTestData** 的 bitset 进行命中判定，不运行时读取纹理 Alpha，也不另造一套命中格式。需要像素命中的 GObject 重写 Godot 的 **_has_point(Vector2 point)**，在其中按 FairyGUI 的 offset、source size、scale 和 bitset 规则返回命中结果。
 * **语言无关的信号分发：** GObject 捕获 _gui_input 后，通过 ClassDB::bind_method 统一以标准 Godot Signal 形式抛出。信号命名采用 `fgui_` 前缀风格，例如 **fgui_click**，避免与 Godot 原生信号和用户脚本命名冲突，同时保持来源清晰。无论是 C# 还是 GDScript 用户，都可以使用标准的 connect 语法监听事件。
## 4. 动效与状态层 (Animation & GTween Core)
**严禁使用 Godot 的 AnimationPlayer 承载 Transition 动效。** 因为 FairyGUI 的动画包含大量动态修改目标值、打断反向播放以及与 Controller (Gear系统) 强联动的硬性业务需求。
 * **核心设计：** 在 GDExtension 中完全独立重写一套纯 C++ 的 **GTween 驱动内核**。
 * **运行机制：** 由一个全局单例（或注册在系统树上的隐式节点）接管系统的 NOTIFICATION_PROCESS。每一帧遍历活跃的 Tweener 链表，完成数值插值计算后，**直接修改目标组件内存中的位置、缩放、透明度属性（set_position, set_modulate 等）**，直接驱动画面刷新。
# 🔍 第二部分：关键技术难点与边界考量
在实现具体组件时，请务必遵守以下已经过团队确立的能力边界定义，不可盲目造轮子：
### 1. 动态文本组件 (GTextField) 的分级优化
FairyGUI 的文本具备富文本、UBB 语法、超链接交互以及复杂的排版特性。
 * **第一阶段文本边界：** 优先支持普通文本、bitmap font 与基础 UBB。普通文本与 bitmap font 由 GTextField 作为 Control 节点自绘，不生成 RichTextLabel 节点。
 * **富文本能力边界：** 更复杂的富文本、超链接交互、图文混排与动态排版在后续阶段接入 Godot 原生 **RichTextLabel** 作为 GTextField 的无 owner 隐式子节点托管。
   * *转换规则：* 在 C++ 中将 FairyGUI 的 `[img]ui://...[/img]` UBB 标签转换为 Godot 支持的 `[img]` 路径，并绑定其 `meta_clicked` 信号以映射超链接点击。
 * **静态文本边界：** 对于极其庞大的 UI 界面（如巨型背包、排行榜）中成百上千个普通的、不带格式的静态 Label，**禁止**生成 RichTextLabel 节点。一律使用底层 Font::draw_string 或 bitmap font 绘制路径在 GTextField 的 _draw() 生命周期内实现裸画。
### 2. 裁剪与遮罩 (Clipping & Masking)
 * **矩形裁剪：** 当 FairyGUI 组件勾选了“溢出隐藏（Overflow）”时，C++ 层直接调用底层的 set_clip_contents(true)。利用 Godot 引擎在 Canvas 层的硬件级裁剪矩阵，不产生额外性能损耗。
 * **不规则遮罩（圆形、自定义形状）：** 由于 Godot 原生的 clip_children 属性在进行形状裁剪时不支持边缘抗锯齿与羽化（Feather），会导致现代高端 UI 边缘出现粗糙毛刺。
   * *实现规范：* 框架在 GDExtension 内部集成一个通用的 **2D Clip Shader (CanvasItemMaterial)**。当组件配置了不规则 Mask 资产时，底层自绘逻辑自动附加该 Shader，在 GPU 着色器阶段直接对遮罩图的 Alpha 通道进行采样混合（COLOR.a *= mask_alpha），完美还原高保真羽化边缘。
### 3. GLoader 统一资源加载
GLoader 保持 FairyGUI Runtime 的统一 Loader 心智：业务通过 url/content 语义加载外部图片、包内图片、包内组件与动图资源，并由 align、vertical_align、fill、shrink_only、auto_size、use_resize 等布局属性控制显示结果。
 * **实现规范：** Godot 侧不设计 GLoader3D，也不为 GLoader 增加 `set_3d_scene`、`get_3d_camera` 等 3D 特化接口。FairyGUI 导出数据中的 Loader3D 类型仍映射为 GLoader 节点，但其能力边界保持在 GLoader 的统一 URL/内容装载模型内。
 * **节点原则：** GLoader 加载包内对象时可持有内部 GObject 子节点；加载纹理类资源时由自身 _draw() 绘制。内部节点属于运行时内容，不制造磁盘中间资源，也不把资源加载策略暴露成 Godot 专用的场景/相机控制 API。
# 🛠️ 第三部分：开发工具与编辑器级支持 (@tool)
为了让游戏逻辑开发者获得所见即所得的极佳体验，本框架必须支持 **Godot 编辑器视口内的实时渲染预览**。为了防止生成的隐式节点对用户的磁盘文件造成不可逆的污染，必须严格执行以下设计约束：
 * **根 GComponent 定义：** 用户可在 Godot 场景中手动创建一个作为入口的 GComponent 节点，并在 Inspector 中配置 package_name、component_name 等引用信息；这个根节点可以保存进 .tscn。它根据配置动态生成的包内 GObject 子节点属于运行时/预览结果，不应保存进 .tscn。
 * **无所有者原则 (No Owner Principle)：** 当框架运行在编辑器环境（Engine::get_singleton()->is_editor_hint() == true）下时，GComponent 会根据用户指定的 Package 资源动态 add_child 生成 GObject 节点树；这些运行时生成的节点以及 RichTextLabel 等隐式子节点，绝不设置 owner。
   * **核心禁令：运行时或编辑器预览动态 add_child(child_node) 后，绝对、严禁设置其 owner 属性！**
   * *底层原理：* Godot 场景序列化机制规定，只有当节点的 owner 指向当前场景的根节点时，Ctrl+S 保存才会将其写入 .tscn 文件。不设 owner，节点在编辑器视口中可以完美绘制与参与输入，但永远不会被意外保存进用户的磁盘文件中。
# 📅 第四部分：研发里程碑与任务分解
开发人员在拿到本计划书后，建议分为以下四个连续的里程碑阶段进行敏捷推进。所有 Milestone 只是内部过程管理概念：完成某一阶段后必须继续推进后续 Runtime、Editor Utility、Sample、性能与兼容性建设，直到形成完整工程可用框架。
| 里程碑阶段 | 核心开发任务 | 验收标准 |
|---|---|---|
| **Milestone 1
(资产管道基础设施)** | 1. 搭建 GDExtension C++ 基础工程与类注册。
2. 实现 `UIPackage.add_package(...)` 直接反序列化 .bytes。
3. 实现 UIPackage、PackageItem、ByteBuffer、AtlasSprite、PixelHitTestData 等基础数据结构。
4. 实现图集元数据解析与运行时 AtlasTexture/NTexture 包装缓存。
5. 实现可选 EditorImportPlugin，用于编辑器识别、预览刷新与依赖追踪，不批量生成多余中间文件。 | 通过 Godot 资源路径、原始字节或自定义加载回调载入 FairyGUI 导出的 .bytes 包与图集，Godot 4.5 下能正确解析包结构、图集切片与 PixelHitTestData；该验收仅限定本里程碑范围，完成后继续推进运行时实例化与渲染。 |
| **Milestone 2
(静态渲染与自适应)** | 1. 实现 GObject(Control) 基类、GComponent 与 @tool 模式编辑器支持。
2. 实现 GImage、GTextField、GButton 等 Control 节点自绘与基础交互，支持九宫格、基础文本与按钮状态切换。
3. 接入 NOTIFICATION_RESIZED 信号，实现 Relations 关联排版系统。 | 游戏未运行状态下，在 Godot 编辑器内手动拖入组件并配置包名，视口内能完美摆放并实时预览 UI 静态画面，且自适应缩放无误；GButton 可用于后续交互和 demo 验证。 |
| **Milestone 3
(交互、文本与 GLoader)** | 1. 使用 PixelHitTestData 重写 _has_point，实现像素级穿透测试。
2. 对接原生事件流并对外抛出标准 Godot Signal。
3. 完成 GTextField 普通文本、bitmap font、基础 UBB。
4. 实现 GLoader 统一资源加载，覆盖外部图片、包内图片、包内组件与动图，并对齐 FairyGUI 的布局属性。 | 点击 UI 组件能正确触发 C#/GDScript 脚本绑定的函数；镂空图透明处可实现穿透点击；基础文本能力可用；GLoader 能统一承载外部图片、包内图片、包内对象与动图资源。 |
| **Milestone 4
(内核灵魂注入与调优)** | 1. 纯 C++ 编写 GTween 驱动管理器。
2. 解析并运行 Transition 动效，打通与 Controller (Gear) 状态机的联动。
3. 实现虚拟列表（Virtual GList），开发有限节点复用池。
4. 自研紧凑型 Godot 专用 demo，用于覆盖导入、静态渲染、交互、文本、GLoader、Transition 与虚拟列表。 | 跑通 FairyGUI 标志性的多状态页面切换、按钮动效；长列表滑动时内存稳定、无卡顿，Draw Call 处于极低范围内；项目拥有自己的紧凑 Godot demo，不依赖 Unity sample 作为最终验收载体。 |
