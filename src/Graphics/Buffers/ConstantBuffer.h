#ifndef ConstantBuffer_h__
#define ConstantBuffer_h__
#include <d3d12.h>
#include <d3dx12.h>
#include <wrl/client.h>

namespace wrl = Microsoft::WRL;

template<class T>
class ConstantBuffer
{
private:
	ConstantBuffer(const ConstantBuffer<T>& rhs);
private:

public:
	ConstantBuffer() {}

};

#endif // ConstantBuffer_h__