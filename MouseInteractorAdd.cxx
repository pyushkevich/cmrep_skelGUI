#include "MouseInteractorAdd.h"	

std::vector<TagInfo> MouseInteractorAdd::vectorTagInfo;
std::vector<TagTriangle> MouseInteractorAdd::vectorTagTriangles;
std::vector<TagPoint> MouseInteractorAdd::vectorTagPoints;
std::vector<std::vector<TagPoint>> MouseInteractorAdd::vectorClassifyPoints;
std::vector<TagEdge> MouseInteractorAdd::vectorTagEdges;
//store all the label info, 0 represent no tag on this point
std::vector<double> MouseInteractorAdd::labelData;
bool MouseInteractorAdd::isSkeleton;
int MouseInteractorAdd::selectedTag;

MouseInteractorAdd::MouseInteractorAdd(){
	selectedTriangle = NULL;
//	qtObject = NULL;
	triCol[0] = 0.2; triCol[1] = 0.2; triCol[2] = 0.7;
}

double MouseInteractorAdd::Distance(double p1[3], double p2[3]){
	return std::sqrt(std::pow(p1[0] - p2[0],2) + std::pow(p1[1] - p2[1], 2) + std::pow(p1[2] - p2[2], 2));
}

void MouseInteractorAdd::SetSelectedTriColor(){
	for(int i = 0; i < vectorTagTriangles.size(); i++){
		if(selectedTriangle == vectorTagTriangles[i].triActor)
			selectedTriangle->GetProperty()->SetColor(1,1,0);
		else
			vectorTagTriangles[i].triActor->GetProperty()->SetColor(triCol);
	}
}

int MouseInteractorAdd::ConstrainEdge(int type1, int type2)
{
	// 1 = Branch point  2 = Free Edge point 3 = Interior point  4 = others
	//return 1;
	if(type1 == 1 && type1 == 1)
		return 3;
	else if(type1 == 2 && type2 == 2)
		return 1;
	else if(type1 == 3 && type2 == 3)
		return 2;
	else if((type1 == 1 && type2 == 2) || (type1 == 2 && type2 == 1))
		return 2;
	else if((type1 == 1 && type2 == 3) || (type1 == 1 && type2 == 3))
		return 2;
	else if((type1 == 2 && type2 == 3) || (type1 == 3 && type2 == 2))
		return 2;
}

int MouseInteractorAdd::PairNumber(int a, int b){
	int a1 = std::min(a,b);
	int b1 = std::max(a,b);
	return (a1 + b1) * (a1 + b1 + 1) / 2.0 + b1;
}

void MouseInteractorAdd::DrawTriangle()
{
	int id1 = triPtIds[0], id2 = triPtIds[1], id3 = triPtIds[2];
	if(id1 == id2 || id1 == id3 || id2 == id3)
		return;
	//get normals
	vtkSmartPointer<vtkPolyDataNormals> normalGenerator = vtkSmartPointer<vtkPolyDataNormals>::New();
	vtkSmartPointer<vtkActor> actor0 = static_cast<vtkActor *>(this->GetDefaultRenderer()->GetActors()->GetItemAsObject(0));
	vtkSmartPointer<vtkDataSet> vtkdata = actor0->GetMapper()->GetInputAsDataSet();	

#if VTK_MAJOR_VERSION <= 5
	normalGenerator->SetInput(vtkPolyData::SafeDownCast(vtkdata));
#else
	normalGenerator->SetInputData(vtkdata);
#endif
	normalGenerator->ComputePointNormalsOn();
	normalGenerator->ComputeCellNormalsOff();
	normalGenerator->Update();
	
	vtkSmartPointer<vtkPolyData> normalPolyData = normalGenerator->GetOutput();
	
	vtkFloatArray* normalDataFloat =
		vtkFloatArray::SafeDownCast(normalPolyData->GetPointData()->GetArray("Normals"));

	if(normalDataFloat)
	{
		int nc = normalDataFloat->GetNumberOfTuples();
		std::cout << "There are " << nc
			<< " components in normalDataFloat" << std::endl;

		float *normal1 = new float;
		normalDataFloat->GetTupleValue(vectorTagPoints[id1].seq, normal1);
		float *normal2 = new float;
		normalDataFloat->GetTupleValue(vectorTagPoints[id2].seq, normal2);
		float *normal3 = new float;
		normalDataFloat->GetTupleValue(vectorTagPoints[id2].seq, normal3);

		float normalAverage[3];
		normalAverage[0] = (normal1[0] + normal2[0] + normal3[0]) / 3.0f;
		normalAverage[1] = (normal1[1] + normal2[1] + normal3[1]) / 3.0f;
		normalAverage[2] = (normal1[2] + normal2[2] + normal3[2]) / 3.0f;

		float d1[3];
		d1[0] = vectorTagPoints[id2].pos[0] - vectorTagPoints[id1].pos[0];
		d1[1] = vectorTagPoints[id2].pos[1] - vectorTagPoints[id1].pos[1];
		d1[2] = vectorTagPoints[id2].pos[2] - vectorTagPoints[id1].pos[2];
		float d2[3];
		d2[0] = vectorTagPoints[id3].pos[0] - vectorTagPoints[id2].pos[0];
		d2[1] = vectorTagPoints[id3].pos[1] - vectorTagPoints[id2].pos[1];
		d2[2] = vectorTagPoints[id3].pos[2] - vectorTagPoints[id2].pos[2];

		float result[3];
		vtkMath::Cross(d1, d2, result);
		vtkMath::Normalize(result);
		vtkMath::Normalize(normalAverage);

		float cos = vtkMath::Dot(result, normalAverage);

		if(cos < 0){//need to swap
			int tempid = triPtIds[1];
			triPtIds[1] = triPtIds[2];
			triPtIds[2] = tempid;
			tempid = id2;
			id2 = id3;
			id3 = tempid;
		}
		std::cout<<"see this result "<<result[0]<<" "<<result[1]<<" "<<result[2]<<std::endl;
		std::cout<<"see this normalAverage "<<normalAverage[0]<<" "<<normalAverage[1]<<" "<<normalAverage[2]<<std::endl;
	}
	//vectorTagPoints[id1]; vectorTagPoints[id2]; vectorTagPoints[id3];
		
	int edgeid1 = PairNumber(triPtIds[0], triPtIds[1]);//id
	int edgeid2 = PairNumber(triPtIds[1], triPtIds[2]);
	int edgeid3 = PairNumber(triPtIds[2], triPtIds[0]);
	std::cout<<"ID "<<id1<<" "<<id2<<" "<<id3<<std::endl;
	//std::cout<<"Pair N "<<edgeid1<<" "<<edgeid2<<" "<<edgeid3<<std::endl;
	int cons1 = ConstrainEdge(vectorTagInfo[vectorTagPoints[id1].typeIndex].tagType, vectorTagInfo[vectorTagPoints[id2].typeIndex].tagType);
	int cons2 = ConstrainEdge(vectorTagInfo[vectorTagPoints[id2].typeIndex].tagType, vectorTagInfo[vectorTagPoints[id3].typeIndex].tagType);
	int cons3 = ConstrainEdge(vectorTagInfo[vectorTagPoints[id1].typeIndex].tagType, vectorTagInfo[vectorTagPoints[id3].typeIndex].tagType);

	if(vectorTagEdges[edgeid1].numEdge >= cons1)
		return;
	else if(vectorTagEdges[edgeid2].numEdge >= cons2)
		return;
	else if(vectorTagEdges[edgeid3].numEdge >= cons3)
		return;

	vectorTagEdges[edgeid1].numEdge++;
	vectorTagEdges[edgeid1].ptId1 = triPtIds[0]; vectorTagEdges[edgeid1].ptId2 = triPtIds[1];
	vectorTagEdges[edgeid2].numEdge++; 
	vectorTagEdges[edgeid2].ptId1 = triPtIds[1]; vectorTagEdges[edgeid2].ptId2 = triPtIds[2];
	vectorTagEdges[edgeid3].numEdge++; 
	vectorTagEdges[edgeid3].ptId1 = triPtIds[2]; vectorTagEdges[edgeid3].ptId2 = triPtIds[0];

	vtkSmartPointer<vtkPoints> pts =
		vtkSmartPointer<vtkPoints>::New();
	for(int i = 0; i < triPtIds.size(); i++){
		pts->InsertNextPoint(vectorTagPoints[triPtIds[i]].pos);
	}

	vtkSmartPointer<vtkTriangle> triangle =
		vtkSmartPointer<vtkTriangle>::New();
	triangle->GetPointIds()->SetId ( 0, 0 );
	triangle->GetPointIds()->SetId ( 1, 1 );
	triangle->GetPointIds()->SetId ( 2, 2 );

	vtkSmartPointer<vtkCellArray> triangles =
		vtkSmartPointer<vtkCellArray>::New();
	triangles->InsertNextCell ( triangle );

	// Create a polydata object
	vtkSmartPointer<vtkPolyData> trianglePolyData =
		vtkSmartPointer<vtkPolyData>::New();

	// Add the geometry and topology to the polydata
	trianglePolyData->SetPoints ( pts );
	trianglePolyData->SetPolys ( triangles );

	// Create mapper and actor
	vtkSmartPointer<vtkPolyDataMapper> mapper =
		vtkSmartPointer<vtkPolyDataMapper>::New();
#if VTK_MAJOR_VERSION <= 5
	mapper->SetInput(trianglePolyData);
#else
	mapper->SetInputData(trianglePolyData);
#endif
	vtkSmartPointer<vtkActor> actor =
		vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	actor->GetProperty()->SetColor(triCol);

	for(int i = 0; i < vectorTagTriangles.size(); i++)
	{
		if(vectorTagTriangles[i].centerPos[0] == actor->GetCenter()[0] &&
			vectorTagTriangles[i].centerPos[1] == actor->GetCenter()[1] &&
			vectorTagTriangles[i].centerPos[2] == actor->GetCenter()[2]){
			return;
			std::cout<<"same tri"<<std::endl;
		}
	}

	TagTriangle tri;
	tri.triActor = actor;
	tri.p1[0] = vectorTagPoints[triPtIds[0]].pos[0]; tri.p1[1] = vectorTagPoints[triPtIds[0]].pos[1]; tri.p1[2] = vectorTagPoints[triPtIds[0]].pos[2];
	tri.p2[0] = vectorTagPoints[triPtIds[1]].pos[0]; tri.p2[1] = vectorTagPoints[triPtIds[1]].pos[1]; tri.p2[2] = vectorTagPoints[triPtIds[1]].pos[2];
	tri.p3[0] = vectorTagPoints[triPtIds[2]].pos[0]; tri.p3[1] = vectorTagPoints[triPtIds[2]].pos[1]; tri.p3[2] = vectorTagPoints[triPtIds[2]].pos[2];
	tri.id1 = triPtIds[0]; tri.id2 = triPtIds[1]; tri.id3 = triPtIds[2];
	tri.centerPos[0] = trianglePolyData->GetCenter()[0];
	tri.centerPos[1] = trianglePolyData->GetCenter()[1];
	tri.centerPos[2] = trianglePolyData->GetCenter()[2];
	tri.seq1 = vectorTagPoints[triPtIds[0]].seq;
	tri.seq2 = vectorTagPoints[triPtIds[1]].seq;
	tri.seq3 = vectorTagPoints[triPtIds[2]].seq;
	vectorTagTriangles.push_back(tri);

	this->GetDefaultRenderer()->AddActor(actor);
}

void MouseInteractorAdd::DeleteTriangle(vtkActor* pickedActor){
	if(pickedActor != NULL)
	{
		pickedActor->GetProperty()->SetColor(0,0,0);
		for(int i = 0; i < vectorTagTriangles.size(); i++)
		{
			TagTriangle tri = vectorTagTriangles[i];
			if(pickedActor == tri.triActor)
			{
				vectorTagEdges[PairNumber(vectorTagTriangles[i].id1, vectorTagTriangles[i].id2)].numEdge--;
				vectorTagEdges[PairNumber(vectorTagTriangles[i].id3, vectorTagTriangles[i].id2)].numEdge--;
				vectorTagEdges[PairNumber(vectorTagTriangles[i].id1, vectorTagTriangles[i].id3)].numEdge--;
				vectorTagTriangles.erase(vectorTagTriangles.begin() + i);
				this->GetDefaultRenderer()->RemoveActor(pickedActor);
				i--;
			}
		}
	}
}
	
vtkActor* MouseInteractorAdd::PickAcotrFromMesh(double pos[3])
{
	vtkSmartPointer<vtkActor> pickedActor
		= vtkSmartPointer<vtkActor>::New();

	vtkActorCollection* actorsCollection = this->GetDefaultRenderer()->GetActors();
	actorsCollection->InitTraversal();

	std::cout<<"number of actors: "<<actorsCollection->GetNumberOfItems()<<std::endl;
	//find the point to delete
	for(vtkIdType i = 0; i < actorsCollection->GetNumberOfItems(); i++)
	{
		vtkActor* nextActor = actorsCollection->GetNextActor();
		double* actorPos = nextActor->GetCenter();
		double dis = Distance(pos,actorPos);

		if(dis < 1.0 && nextActor){
			pickedActor = nextActor;
			return pickedActor;
		}
	}
	return NULL;
}

void MouseInteractorAdd::copyEdgeBtoA(int a, int b){
	vectorTagEdges[a].constrain = vectorTagEdges[b].constrain;
	vectorTagEdges[a].numEdge = vectorTagEdges[b].numEdge;
}

int MouseInteractorAdd::deleteEdgeHelper(int id1, int id2, int seq)
{
	if(id1 < seq && id2 < seq)
		return PairNumber(id1, id2);
	else if(id1 < seq && id2 > seq)
		return PairNumber(id1, id2-1);
	else if(id1 > seq && id2 < seq)
		return PairNumber(id1-1, id2);
	else if(id1 > seq && id2 > seq)
		return PairNumber(id1 - 1 , id2 - 1);
}

int MouseInteractorAdd::deleteEdgeHelper2(int id, int seq)
{
	if(id > seq)
		return id - 1;
	else
		return id;
}

void MouseInteractorAdd::deleteEdge(int seq)
{
	std::vector<TagEdge> temp;
	std::vector<int> zeroEdgeId;
	for(int i = 0; i < vectorTagTriangles.size(); i++)
	{			
		int id1 = vectorTagTriangles[i].id1, id2 = vectorTagTriangles[i].id2, id3 = vectorTagTriangles[i].id3;
		int nori;			
		int n;
		TagEdge e1;
		if(id1 == seq || id2 == seq || id3 == seq)//if the point is belongs to triangle
		{
			int d1, d2;
			if(id1 == seq)
			{
				d1 = PairNumber(id1, id2);
				d2 = PairNumber(id1, id3);
				nori = PairNumber(id2,id3);
				n = deleteEdgeHelper(id2,id3,seq);
			}
			else if(id2 == seq)
			{
				d1 = PairNumber(id2, id3);
				d2 = PairNumber(id2, id1);
				nori = PairNumber(id1,id3);
				n = deleteEdgeHelper(id1,id3,seq);
			}
			else if(id3 == seq)
			{
				d1 = PairNumber(id3, id1);
				d2 = PairNumber(id3, id2);
				nori = PairNumber(id1,id2);
				n = deleteEdgeHelper(id1,id2,seq);
			}

			e1.seq = n;
			e1.constrain = vectorTagEdges[nori].constrain;
			e1.numEdge = --vectorTagEdges[nori].numEdge;
			e1.ptId1 = deleteEdgeHelper2(vectorTagEdges[nori].ptId1, seq);
			e1.ptId2 = deleteEdgeHelper2(vectorTagEdges[nori].ptId2, seq);
			temp.push_back(e1);
			zeroEdgeId.push_back(nori);
			zeroEdgeId.push_back(d1);
			zeroEdgeId.push_back(d2);
			//std::cout<<"0 id"<<nori<<" "<<d1<<" "<<d2<<std::endl;
			this->GetDefaultRenderer()->RemoveActor(vectorTagTriangles[i].triActor);
			vectorTagTriangles.erase(vectorTagTriangles.begin() + i);
			i--;
		}
		else
		{
			int minId = std::min(id1, std::min(id2, id3));
			int maxId = std::max(id1, std::max(id2, id3));
			if(maxId > seq)
			{

				for(int j = 0; j < 3; j++)
				{
					if(j == 0){
						nori = PairNumber(id1,id2);
						n = deleteEdgeHelper(id1,id2,seq);
					}
					else if(j == 1){
						nori = PairNumber(id3,id2);
						n = deleteEdgeHelper(id3,id2,seq);
					}
					else if(j == 2){
						nori = PairNumber(id3,id1);
						n = deleteEdgeHelper(id3,id1,seq);
					}
					e1.seq = n;
					e1.constrain = vectorTagEdges[nori].constrain;
					e1.numEdge = vectorTagEdges[nori].numEdge;
					e1.ptId1 = deleteEdgeHelper2(vectorTagEdges[nori].ptId1, seq);
					e1.ptId2 = deleteEdgeHelper2(vectorTagEdges[nori].ptId2, seq);
					temp.push_back(e1);
					zeroEdgeId.push_back(nori);
				}
			}
			vectorTagTriangles[i].id1 = deleteEdgeHelper2(id1,seq);
			vectorTagTriangles[i].id2 = deleteEdgeHelper2(id2,seq);
			vectorTagTriangles[i].id3 = deleteEdgeHelper2(id3,seq);
		}			
	}

	for (int i = 0; i < zeroEdgeId.size(); i++)
	{
		int id = zeroEdgeId[i];
		vectorTagEdges[id].constrain = vectorTagEdges[id].numEdge = vectorTagEdges[id].ptId1 = vectorTagEdges[id].ptId2 = 0;
	}

	for(int j = 0; j < temp.size(); j++)
	{
		int nu = temp[j].seq;
		vectorTagEdges[nu].constrain = temp[j].constrain;
		vectorTagEdges[nu].numEdge = temp[j].numEdge;
		vectorTagEdges[nu].ptId1 = temp[j].ptId1;
		vectorTagEdges[nu].ptId2 = temp[j].ptId2;
	}
		
}

vtkStandardNewMacro(MouseInteractorAdd);