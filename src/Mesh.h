#ifndef __MESH_H_
#define __MESH_H_

class Mesh {
public:
    Mesh();
	virtual ~Mesh();
    std::vector<unsigned int> eleBuf;
    std::vector<float> posBuf;
    std::vector<float> norBuf;
    std::vector<float> texBuf;
}

#endif // __MESH_H_
