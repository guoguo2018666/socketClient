#ifndef _CELL_STREAM_HPP_
#define _CELL_STREAM_HPP_
#include<cstdint>
#include <memory>
//字节流byte
class CELLStream
{
public:
	CELLStream(int nSize = 1024) {
		_nSumSize = nSize;
		_pMsgBuf = new char[_nSumSize];
		_bDelete = true;
	}

	CELLStream(char* pData,int nSize,bool bDelete = false) {
		_nSumSize = nSize;
		_pMsgBuf = pData;
		_bDelete = bDelete;
	}

	virtual ~CELLStream() {
		if (!_pMsgBuf && _bDelete) {
			delete[] _pMsgBuf;
		}
	}

	char* getData() {
		return _pMsgBuf;
	}

	int getDataLength() {
		return  _nWritePos;
	}

	void setWritePos(int n) {
		_nWritePos = n;
	}

	int getWritePos() {
		return _nWritePos;
	}

	void push(int n) {
		_nWritePos += n;
	}

	void pop(int n) {
		_nReadPos += n;
	}


	//是否有n的空间可读
	bool canRead(int n) {
		return _nSumSize - _nReadPos >= n;
	}
	
	//read
	template<typename T>
	bool read(T& n,bool bOffset = true) {
		//计算读取数据的长度
		auto nLength = sizeof(T);

		/*if (_nReadPos > _nWritePos) {
		return 0;
		}*/
		//int nCanReadSize = _nWritePos - _nReadPos;

		if (_nReadPos + nLength <= _nSumSize) {
			memcpy(&n, _pMsgBuf + _nReadPos, nLength);

			if (bOffset) {
				_nReadPos += nLength;
			}
			
			return true;
		}
		return false;	
	}

	template<typename T>
	uint32_t readArray(T* pData, uint32_t nLength) {

		//读取数据元素个数
		uint32_t nReadLength = 0;
		bool bReadLength = read(nReadLength, false);
		if (nReadLength <= nLength) {
			//计算要写入数据大小
			auto nReadSize = sizeof(T)*nReadLength;

			if (_nReadPos + nReadSize + sizeof(uint32_t) <= _nSumSize) {
				_nReadPos += sizeof(uint32_t);
				memcpy(pData, _pMsgBuf + _nReadPos , nReadSize);
				_nReadPos += nReadSize;
				return nReadLength;
			}
		}//if

		return 0;
	}
	template<typename T>
	T onlyRead(T n) {
		 read(n, false);
		 return n;
	}

	int8_t readInt8(int8_t n = 0) {
		read(n);
		return n;
	}

	//short
	int16_t readInt16(int16_t n = 0) {
		read(n);
		return n;;
	}
	//int
	int32_t readInt32(int32_t n = 0) {
		read(n);
		return n;
	}
	//float
	float readFloat(float n = 0) {
		read(n);
		return n;
	}
	//double
	double readDouble(double n = 0) {
		read(n);
		return n;
	}

	//write
	template<typename T>
	bool write(T n) {
		size_t nDataLength = sizeof(T);
		int nLeftSize = _nSumSize - _nWritePos;

		//char data[32];
		//sprintf(data, "%d", n);
		//判断能不能写入
		if (nDataLength <= nLeftSize) {
			std::cout << "writePosbegin[" << _nWritePos << "]" << std::endl;
			memcpy(_pMsgBuf + _nWritePos, &n, nDataLength);
			_nWritePos += nDataLength;
			return true;
		}
		return false;
	}
	template<typename T>
	bool writeArray(T* pData, uint32_t nLength) {
		//计算要写入数据点额大小
		auto nWriteLength = sizeof(T)*nLength + sizeof(uint32_t);

		int nLeftSize = _nSumSize - _nWritePos;

		//判断能不能写入
		if (nWriteLength <= nLeftSize) {
			//写入数组的成员个数
			writeInt32(nLength);

			//写入实际数据
			memcpy(_pMsgBuf + _nWritePos, pData, nWriteLength);
			_nWritePos += sizeof(T)*nLength;
			return true;
		}
		return false;
	}

	bool writeInt8(int8_t n) {
		return write(n);
	}
	bool writeInt16(int16_t n) {
		return write(n);
	}
	bool writeInt32(int32_t n) {
		return write(n);
	}
	bool writeFloat(float n) {
		return write(n);
	}
	bool writeDouble(double n) {
		return write(n);
	}
//protected:
private:
	//缓冲区
	char * _pMsgBuf = nullptr;

	//已写入的尾部位置
	int _nWritePos = 0;

	//已读取数据的尾部位置
	int _nReadPos = 0;

	//缓冲区总的空间大小
	int _nSumSize = 0;

	//缓冲区是否被放满的次数
	int _nBuffFullCount = 0;

	//外部传入的缓冲区是否需要删除
	bool _bDelete = true;

};



#endif 