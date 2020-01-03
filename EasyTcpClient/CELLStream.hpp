#ifndef _CELL_STREAM_HPP_
#define _CELL_STREAM_HPP_
#include<cstdint>
#include <memory>
//�ֽ���byte
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


	//�Ƿ���n�Ŀռ�ɶ�
	bool canRead(int n) {
		return _nSumSize - _nReadPos >= n;
	}
	
	//read
	template<typename T>
	bool read(T& n,bool bOffset = true) {
		//�����ȡ���ݵĳ���
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

		//��ȡ����Ԫ�ظ���
		uint32_t nReadLength = 0;
		bool bReadLength = read(nReadLength, false);
		if (nReadLength <= nLength) {
			//����Ҫд�����ݴ�С
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
		//�ж��ܲ���д��
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
		//����Ҫд�����ݵ���С
		auto nWriteLength = sizeof(T)*nLength + sizeof(uint32_t);

		int nLeftSize = _nSumSize - _nWritePos;

		//�ж��ܲ���д��
		if (nWriteLength <= nLeftSize) {
			//д������ĳ�Ա����
			writeInt32(nLength);

			//д��ʵ������
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
	//������
	char * _pMsgBuf = nullptr;

	//��д���β��λ��
	int _nWritePos = 0;

	//�Ѷ�ȡ���ݵ�β��λ��
	int _nReadPos = 0;

	//�������ܵĿռ��С
	int _nSumSize = 0;

	//�������Ƿ񱻷����Ĵ���
	int _nBuffFullCount = 0;

	//�ⲿ����Ļ������Ƿ���Ҫɾ��
	bool _bDelete = true;

};



#endif 