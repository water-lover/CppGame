#include "viewmodel/SpiritVM.hpp"

// SpiritVM 现在由 App 通过 setter 注入图片指针，
// 不再直接调用 AssetManager（遵循 ViewModel 不依赖 Resource 的规则）。
// 本文件保留以备后续扩展（如图片缩放预处理）。
