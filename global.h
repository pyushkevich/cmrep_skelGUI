#ifndef _GLOBAL_H_
#define _GLOBAL_H_
#include <QColor>
#include <vtkActor.h>
#include <vector>

//from pyushkevich/cmrep/src/MeshTransversal.h 
// Constant used for unreasonable size_t objects
#define NOID 0xffffffff


/**
 * A representation of a triangle in a mesh
 */
struct Triangle
{
	// Index of the triangle's vertices
	size_t vertices[3];

	// Index of the neighbors
	size_t neighbors[3];

	// Optional label of the triangle. Will be propagated to children
	size_t label;

	// Each edge is associated with an index of the vertex opposite to it.
	// This value tells us for each edge its index in the adjacent triangle
	short nedges[3];

	// Initializes to dummy values
	Triangle()
	{
		vertices[0] = vertices[1] = vertices[2] = NOID;
		neighbors[0] = neighbors[1] = neighbors[2] = NOID;
		nedges[0] = nedges[1] = nedges[2] = -1;
		label = NOID;
	}
};


struct TagInfo
{
	std::string tagName;
	int tagType; // 1 = Branch point  2 = Free Edge point 3 = Interior point  4 = others
	int tagColor[3];//for color
	QColor qc;//for color	
	int tagIndex;//the index of the tag(1-10)
};

struct LabelTriangle
{
	std::string labelName; // name of the triangle label
	QColor labelColor; // color of the triangle label
};

struct TagTriangle
{
	vtkActor *triActor;
	double p1[3], p2[3], p3[3];
	double centerPos[3];
	int id1, id2, id3;//for point index in tagPoint
	int seq1, seq2, seq3;//for point index in all vertices  on skeleton
	int index;//the triangle label index
};


struct TagPoint
{
	vtkActor* actor;
	std::string typeName;
	int typeIndex;//tag index
	int comboBoxIndex;//index in combobox
	double radius;//radius of that points
	int seq;//the sequence in all vertices  on skeleton
	double pos[3];
	int ptIndex;
};	


struct TagEdge
{
	int ptId1;
	int ptId2;
	int constrain;// store the constraint of these two points
	int numEdge;//store how many edge it has
	int seq;//use of deletion
};

struct TagAction
{
	int action;
	double pos[3];
	int triIndex;//for triangle label
	TagPoint pointInfo;
	TagTriangle triangleInfo;
	int ptIndex;
	int ptOldSeq;
};

class Global
{
public:
	//Store Tag Information
	static std::vector<TagInfo> vectorTagInfo;
	//Store Triangle label information
	static std::vector<LabelTriangle> vectorLabelInfo;
	//Store Triangle information
	static std::vector<TagTriangle> vectorTagTriangles;
	//Store Points information
	static std::vector<TagPoint> vectorTagPoints;	
	//Store Edge information
	static std::vector<TagEdge> vectorTagEdges;

	//store all the label info, 0 represent no tag on this point
	static std::vector<double> labelData;
	static std::vector<vtkActor*> triNormalActors;
	static bool isSkeleton;
	static int selectedTag;
	static double targetReduction;
	static double featureAngle;
	static double tagRadius;

	static double triCol[3];
	static double backCol[3];

	static bool decimateMode;
};

#endif
