#include "MD5Model.h"
#include <fstream>

MD5Model::MD5Model()
{
}

MD5Model::~MD5Model()
{
}

bool MD5Model::LoadMD5Model(ID3D12Device6* device, ID3D12GraphicsCommandList6* commandList, 
    std::wstring filename, std::vector<std::wstring>* texFileNameArray)
{

    std::wifstream fileIn(filename.c_str());        // Open file

    std::wstring checkString;                        // Stores the next string from our file

    if (fileIn)                                        // Check if the file was opened
    {
        while (fileIn)                                // Loop until the end of the file is reached
        {
            fileIn >> checkString;                    // Get next string from file

            if (checkString == L"MD5Version")        // Get MD5 version (this function supports version 10)
            {
                /*fileIn >> checkString;
                MessageBox(0, checkString.c_str(),    //display message
                L"MD5Version", MB_OK);*/
            }
            else if (checkString == L"commandline")
            {
                std::getline(fileIn, checkString);    // Ignore the rest of this line
            }
            else if (checkString == L"numJoints")
            {
                fileIn >> m_model.numJoints;        // Store number of joints
            }
            else if (checkString == L"numMeshes")
            {
                fileIn >> m_model.numSubsets;        // Store number of meshes or subsets which we will call them
            }
            else if (checkString == L"joints")
            {
                MD5Model::Joint tempJoint;

                fileIn >> checkString;                // Skip the "{"

                for (int i = 0; i < m_model.numJoints; i++)
                {
                    fileIn >> tempJoint.name;        // Store joints name
                    // Sometimes the names might contain spaces. If that is the case, we need to continue
                    // to read the name until we get to the closing " (quotation marks)
                    if (tempJoint.name[tempJoint.name.size() - 1] != '"')
                    {
                        wchar_t checkChar;
                        bool jointNameFound = false;
                        while (!jointNameFound)
                        {
                            checkChar = fileIn.get();

                            if (checkChar == '"')
                                jointNameFound = true;

                            tempJoint.name += checkChar;
                        }
                    }

                    fileIn >> tempJoint.parentID;    // Store Parent joint's ID

                    fileIn >> checkString;            // Skip the "("

                    // Store position of this joint (swap y and z axis if model was made in RH Coord Sys)
                    fileIn >> tempJoint.pos.x >> tempJoint.pos.z >> tempJoint.pos.y;

                    fileIn >> checkString >> checkString;    // Skip the ")" and "("

                    // Store orientation of this joint
                    fileIn >> tempJoint.orientation.x >> tempJoint.orientation.z >> tempJoint.orientation.y;

                    // Remove the quotation marks from joints name
                    tempJoint.name.erase(0, 1);
                    tempJoint.name.erase(tempJoint.name.size()-1, 1);

                    // Compute the w axis of the quaternion (The MD5 model uses a 3D vector to describe the
                    // direction the bone is facing. However, we need to turn this into a quaternion, and the way
                    // quaternions work, is the xyz values describe the axis of rotation, while the w is a value
                    // between 0 and 1 which describes the angle of rotation)
                    float t = 1.0f - (tempJoint.orientation.x * tempJoint.orientation.x)
                        - (tempJoint.orientation.y * tempJoint.orientation.y)
                        - (tempJoint.orientation.z * tempJoint.orientation.z);
                    if (t < 0.0f)
                    {
                        tempJoint.orientation.w = 0.0f;
                    }
                    else
                    {
                        tempJoint.orientation.w = -sqrtf(t);
                    }

                    std::getline(fileIn, checkString);        // Skip rest of this line

                    m_model.joints.push_back(tempJoint);    // Store the joint into this models joint vector
                }

                fileIn >> checkString;                    // Skip the "}"
            }
            else if (checkString == L"mesh")
            {
                
                int numVerts, numTris, numWeights;

                // Push back the temp subset into the models subset vector
                m_model.subsets.emplace_back();
                MD5Model::ModelSubset& subset = m_model.subsets.back();

                fileIn >> checkString;                    // Skip the "{"

                fileIn >> checkString;
                while (checkString != L"}")            // Read until '}'
                {
                    // In this lesson, for the sake of simplicity, we will assume a textures filename is givin here.
                    // Usually though, the name of a material (stored in a material library. Think back to the lesson on
                    // loading .obj files, where the material library was contained in the file .mtl) is givin. Let this
                    // be an exercise to load the material from a material library such as obj's .mtl file, instead of
                    // just the texture like we will do here.
                    if (checkString == L"shader")        // Load the texture or material
                    {
                        std::wstring fileNamePath;
                        fileIn >> fileNamePath;            // Get texture's filename

                        // Take spaces into account if filename or material name has a space in it
                        if (fileNamePath[fileNamePath.size() - 1] != '"')
                        {
                            wchar_t checkChar;
                            bool fileNameFound = false;
                            while (!fileNameFound)
                            {
                                checkChar = fileIn.get();

                                if (checkChar == '"')
                                    fileNameFound = true;

                                fileNamePath += checkChar;
                            }
                        }

                        // Remove the quotation marks from texture path
                        fileNamePath.erase(0, 1);
                        fileNamePath.erase(fileNamePath.size() - 1, 1);

                        //check if this texture has already been loaded
                        bool alreadyLoaded = false;
                        for (int i = 0; i < (*texFileNameArray).size(); ++i)
                        {
                            if (fileNamePath == (*texFileNameArray)[i])
                            {
                                alreadyLoaded = true;
                                subset.texArrayIndex = i;
                            }
                        }

                        //if the texture is not already loaded, load it now
                        if (!alreadyLoaded)
                        {

                            (*texFileNameArray).push_back(fileNamePath.c_str());


                            // read in textures


                            //ID3D12Resource* tempMeshSRV;
                            //hr = D3DX11CreateShaderResourceViewFromFile(d3d11Device, fileNamePath.c_str(),
                            //    NULL, NULL, &tempMeshSRV, NULL);
                            //if (SUCCEEDED(hr))
                            //{
                            //    texFileNameArray.push_back(fileNamePath.c_str());
                            //    subset.texArrayIndex = shaderResourceViewArray.size();
                            //    shaderResourceViewArray.push_back(tempMeshSRV);
                            //}
                            //else
                            //{
                            //    MessageBox(0, fileNamePath.c_str(),        //display message
                            //        L"Could Not Open:", MB_OK);
                            //    return false;
                            //}
                        }

                        std::getline(fileIn, checkString);                // Skip rest of this line
                    }
                    else if (checkString == L"numverts")
                    {
                        fileIn >> numVerts;                                // Store number of vertices

                        std::getline(fileIn, checkString);                // Skip rest of this line

                        for (int i = 0; i < numVerts; i++)
                        {
                            MD5Model::Vertex tempVert;

                            fileIn >> checkString                        // Skip "vert # ("
                                >> checkString
                                >> checkString;

                            fileIn >> tempVert.texCoord.x                // Store tex coords
                                >> tempVert.texCoord.y;

                            fileIn >> checkString;                        // Skip ")"

                            fileIn >> tempVert.StartWeight;                // Index of first weight this vert will be weighted to

                            fileIn >> tempVert.WeightCount;                // Number of weights for this vertex

                            std::getline(fileIn, checkString);            // Skip rest of this line

                            subset.vertices.push_back(tempVert);        // Push back this vertex into subsets vertex vector
                        }
                    }
                    else if (checkString == L"numtris")
                    {
                        fileIn >> numTris;
                        subset.numTriangles = numTris;

                        std::getline(fileIn, checkString);                // Skip rest of this line

                        for (int i = 0; i < numTris; i++)                // Loop through each triangle
                        {
                            DWORD tempIndex;
                            fileIn >> checkString;                        // Skip "tri"
                            fileIn >> checkString;                        // Skip tri counter

                            for (int k = 0; k < 3; k++)                    // Store the 3 indices
                            {
                                fileIn >> tempIndex;
                                subset.indices.push_back(tempIndex);
                            }

                            std::getline(fileIn, checkString);            // Skip rest of this line
                        }
                    }
                    else if (checkString == L"numweights")
                    {
                        fileIn >> numWeights;

                        std::getline(fileIn, checkString);                // Skip rest of this line

                        for (int i = 0; i < numWeights; i++)
                        {
                            MD5Model::Weight tempWeight;
                            fileIn >> checkString >> checkString;        // Skip "weight #"

                            fileIn >> tempWeight.jointID;                // Store weight's joint ID

                            fileIn >> tempWeight.bias;                    // Store weight's influence over a vertex

                            fileIn >> checkString;                        // Skip "("

                            fileIn >> tempWeight.pos.x                    // Store weight's pos in joint's local space
                                >> tempWeight.pos.z
                                >> tempWeight.pos.y;

                            std::getline(fileIn, checkString);            // Skip rest of this line

                            subset.weights.push_back(tempWeight);        // Push back tempWeight into subsets Weight array
                        }

                    }
                    else
                        std::getline(fileIn, checkString);                // Skip anything else

                    fileIn >> checkString;                                // Skip "}"
                }

                //*** find each vertex's position using the joints and weights ***//
                for (int i = 0; i < subset.vertices.size(); ++i)
                {
                    MD5Model::Vertex tempVert = subset.vertices[i];
                    tempVert.pos = XMFLOAT3(0, 0, 0);    // Make sure the vertex's pos is cleared first

                    // Sum up the joints and weights information to get vertex's position
                    for (int j = 0; j < tempVert.WeightCount; ++j)
                    {
                        MD5Model::Weight tempWeight = subset.weights[tempVert.StartWeight + j];
                        MD5Model::Joint tempJoint = m_model.joints[tempWeight.jointID];

                        // Convert joint orientation and weight pos to vectors for easier computation
                        // When converting a 3d vector to a quaternion, you should put 0 for "w", and
                        // When converting a quaternion to a 3d vector, you can just ignore the "w"
                        XMVECTOR tempJointOrientation = XMVectorSet(tempJoint.orientation.x, tempJoint.orientation.y, tempJoint.orientation.z, tempJoint.orientation.w);
                        XMVECTOR tempWeightPos = XMVectorSet(tempWeight.pos.x, tempWeight.pos.y, tempWeight.pos.z, 0.0f);

                        // We will need to use the conjugate of the joint orientation quaternion
                        // To get the conjugate of a quaternion, all you have to do is inverse the x, y, and z
                        XMVECTOR tempJointOrientationConjugate = XMVectorSet(-tempJoint.orientation.x, -tempJoint.orientation.y, -tempJoint.orientation.z, tempJoint.orientation.w);

                        // Calculate vertex position (in joint space, eg. rotate the point around (0,0,0)) for this weight using the joint orientation quaternion and its conjugate
                        // We can rotate a point using a quaternion with the equation "rotatedPoint = quaternion * point * quaternionConjugate"
                        XMFLOAT3 rotatedPoint;
                        XMStoreFloat3(&rotatedPoint, XMQuaternionMultiply(XMQuaternionMultiply(tempJointOrientation, tempWeightPos), tempJointOrientationConjugate));

                        // Now move the verices position from joint space (0,0,0) to the joints position in world space, taking the weights bias into account
                        // The weight bias is used because multiple weights might have an effect on the vertices final position. Each weight is attached to one joint.
                        tempVert.pos.x += (tempJoint.pos.x + rotatedPoint.x) * tempWeight.bias;
                        tempVert.pos.y += (tempJoint.pos.y + rotatedPoint.y) * tempWeight.bias;
                        tempVert.pos.z += (tempJoint.pos.z + rotatedPoint.z) * tempWeight.bias;

                        // Basically what has happened above, is we have taken the weights position relative to the joints position
                        // we then rotate the weights position (so that the weight is actually being rotated around (0, 0, 0) in world space) using
                        // the quaternion describing the joints rotation. We have stored this rotated point in rotatedPoint, which we then add to
                        // the joints position (because we rotated the weight's position around (0,0,0) in world space, and now need to translate it
                        // so that it appears to have been rotated around the joints position). Finally we multiply the answer with the weights bias,
                        // or how much control the weight has over the final vertices position. All weight's bias effecting a single vertex's position
                        // must add up to 1.
                    }

                    subset.positions.push_back(tempVert.pos);            // Store the vertices position in the position vector instead of straight into the vertex vector
                    // since we can use the positions vector for certain things like collision detection or picking
                    // without having to work with the entire vertex structure.
                }

                // Put the positions into the vertices for this subset
                for (int i = 0; i < subset.vertices.size(); i++)
                {
                    subset.vertices[i].pos = subset.positions[i];
                }

                //*** Calculate vertex normals using normal averaging ***///
                std::vector<XMFLOAT3> tempNormal;

                //normalized and unnormalized normals
                XMFLOAT3 unnormalized = XMFLOAT3(0.0f, 0.0f, 0.0f);

                //Used to get vectors (sides) from the position of the verts
                float vecX, vecY, vecZ;

                //Two edges of our triangle
                XMVECTOR edge1 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
                XMVECTOR edge2 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

                //Compute face normals
                for (int i = 0; i < subset.numTriangles; ++i)
                {
                    //Get the vector describing one edge of our triangle (edge 0,2)
                    vecX = subset.vertices[subset.indices[(i * 3)]].pos.x - subset.vertices[subset.indices[(i * 3) + 2]].pos.x;
                    vecY = subset.vertices[subset.indices[(i * 3)]].pos.y - subset.vertices[subset.indices[(i * 3) + 2]].pos.y;
                    vecZ = subset.vertices[subset.indices[(i * 3)]].pos.z - subset.vertices[subset.indices[(i * 3) + 2]].pos.z;
                    edge1 = XMVectorSet(vecX, vecY, vecZ, 0.0f);    //Create our first edge

                    //Get the vector describing another edge of our triangle (edge 2,1)
                    vecX = subset.vertices[subset.indices[(i * 3) + 2]].pos.x - subset.vertices[subset.indices[(i * 3) + 1]].pos.x;
                    vecY = subset.vertices[subset.indices[(i * 3) + 2]].pos.y - subset.vertices[subset.indices[(i * 3) + 1]].pos.y;
                    vecZ = subset.vertices[subset.indices[(i * 3) + 2]].pos.z - subset.vertices[subset.indices[(i * 3) + 1]].pos.z;
                    edge2 = XMVectorSet(vecX, vecY, vecZ, 0.0f);    //Create our second edge

                    //Cross multiply the two edge vectors to get the un-normalized face normal
                    XMStoreFloat3(&unnormalized, XMVector3Cross(edge1, edge2));

                    tempNormal.push_back(unnormalized);
                }

                //Compute vertex normals (normal Averaging)
                XMVECTOR normalSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
                int facesUsing = 0;
                float tX, tY, tZ;    //temp axis variables

                //Go through each vertex
                for (int i = 0; i < subset.vertices.size(); ++i)
                {
                    //Check which triangles use this vertex
                    for (int j = 0; j < subset.numTriangles; ++j)
                    {
                        if (subset.indices[j * 3] == i ||
                            subset.indices[(j * 3) + 1] == i ||
                            subset.indices[(j * 3) + 2] == i)
                        {
                            tX = XMVectorGetX(normalSum) + tempNormal[j].x;
                            tY = XMVectorGetY(normalSum) + tempNormal[j].y;
                            tZ = XMVectorGetZ(normalSum) + tempNormal[j].z;

                            normalSum = XMVectorSet(tX, tY, tZ, 0.0f);    //If a face is using the vertex, add the unormalized face normal to the normalSum

                            facesUsing++;
                        }
                    }

                    //Get the actual normal by dividing the normalSum by the number of faces sharing the vertex
                    normalSum = normalSum / facesUsing;

                    //Normalize the normalSum vector
                    normalSum = XMVector3Normalize(normalSum);

                    //Store the normal and tangent in our current vertex
                    subset.vertices[i].normal.x = -XMVectorGetX(normalSum);
                    subset.vertices[i].normal.y = -XMVectorGetY(normalSum);
                    subset.vertices[i].normal.z = -XMVectorGetZ(normalSum);

                    //Clear normalSum, facesUsing for next vertex
                    normalSum = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
                    facesUsing = 0;
                }
             
                CreateVertexBuffers(device, commandList, subset);

            }
        }
    }
    else
    {

        // create message
        std::wstring message = L"Could not open: ";
        message += filename;

        MessageBox(0, message.c_str(),    // display message
            L"Error", MB_OK);

        return false;
    }

    return true;
}

bool MD5Model::LoadMD5Anim(std::wstring filename)
{
    ModelAnimation tempAnim;                        // Temp animation to later store in our model's animation array

    std::wifstream fileIn(filename.c_str());        // Open file

    std::wstring checkString;                        // Stores the next string from our file

    if (fileIn)                                        // Check if the file was opened
    {
        while (fileIn)                                // Loop until the end of the file is reached
        {
            fileIn >> checkString;                    // Get next string from file

            if (checkString == L"MD5Version")        // Get MD5 version (this function supports version 10)
            {
                fileIn >> checkString;
                /*MessageBox(0, checkString.c_str(),    //display message
                L"MD5Version", MB_OK);*/
            }
            else if (checkString == L"commandline")
            {
                std::getline(fileIn, checkString);    // Ignore the rest of this line
            }
            else if (checkString == L"numFrames")
            {
                fileIn >> tempAnim.numFrames;                // Store number of frames in this animation
            }
            else if (checkString == L"numJoints")
            {
                fileIn >> tempAnim.numJoints;                // Store number of joints (must match .md5mesh)
            }
            else if (checkString == L"frameRate")
            {
                fileIn >> tempAnim.frameRate;                // Store animation's frame rate (frames per second)
            }
            else if (checkString == L"numAnimatedComponents")
            {
                fileIn >> tempAnim.numAnimatedComponents;    // Number of components in each frame section
            }
            else if (checkString == L"hierarchy")
            {
                fileIn >> checkString;                // Skip opening bracket "{"

                for (int i = 0; i < tempAnim.numJoints; i++)    // Load in each joint
                {
                    AnimJointInfo tempJoint;

                    fileIn >> tempJoint.name;        // Get joints name
                    // Sometimes the names might contain spaces. If that is the case, we need to continue
                    // to read the name until we get to the closing " (quotation marks)
                    if (tempJoint.name[tempJoint.name.size() - 1] != '"')
                    {
                        wchar_t checkChar;
                        bool jointNameFound = false;
                        while (!jointNameFound)
                        {
                            checkChar = fileIn.get();

                            if (checkChar == '"')
                                jointNameFound = true;

                            tempJoint.name += checkChar;
                        }
                    }

                    // Remove the quotation marks from joints name
                    tempJoint.name.erase(0, 1);
                    tempJoint.name.erase(tempJoint.name.size() - 1, 1);

                    fileIn >> tempJoint.parentID;            // Get joints parent ID
                    fileIn >> tempJoint.flags;                // Get flags
                    fileIn >> tempJoint.startIndex;            // Get joints start index

                    // Make sure the joint exists in the model, and the parent ID's match up
                    // because the bind pose (md5mesh) joint hierarchy and the animations (md5anim)
                    // joint hierarchy must match up
                    bool jointMatchFound = false;
                    for (int k = 0; k < m_model.numJoints; k++)
                    {
                        if (m_model.joints[k].name == tempJoint.name)
                        {
                            if (m_model.joints[k].parentID == tempJoint.parentID)
                            {
                                jointMatchFound = true;
                                tempAnim.jointInfo.push_back(tempJoint);
                            }
                        }
                    }
                    if (!jointMatchFound)                    // If the skeleton system does not match up, return false
                        return false;                        // You might want to add an error message here

                    std::getline(fileIn, checkString);        // Skip rest of this line
                }
            }
            else if (checkString == L"bounds")            // Load in the AABB for each animation
            {
                fileIn >> checkString;                        // Skip opening bracket "{"

                for (int i = 0; i < tempAnim.numFrames; i++)
                {
                    BoundingBox tempBB;

                    fileIn >> checkString;                    // Skip "("
                    fileIn >> tempBB.min.x >> tempBB.min.z >> tempBB.min.y;
                    fileIn >> checkString >> checkString;    // Skip ") ("
                    fileIn >> tempBB.max.x >> tempBB.max.z >> tempBB.max.y;
                    fileIn >> checkString;                    // Skip ")"

                    tempAnim.frameBounds.push_back(tempBB);
                }
            }
            else if (checkString == L"baseframe")            // This is the default position for the animation
            {                                                // All frames will build their skeletons off this
                fileIn >> checkString;                        // Skip opening bracket "{"

                for (int i = 0; i < tempAnim.numJoints; i++)
                {
                    Joint tempBFJ;

                    fileIn >> checkString;                        // Skip "("
                    fileIn >> tempBFJ.pos.x >> tempBFJ.pos.z >> tempBFJ.pos.y;
                    fileIn >> checkString >> checkString;        // Skip ") ("
                    fileIn >> tempBFJ.orientation.x >> tempBFJ.orientation.z >> tempBFJ.orientation.y;
                    fileIn >> checkString;                        // Skip ")"

                    tempAnim.baseFrameJoints.push_back(tempBFJ);
                }
            }
            else if (checkString == L"frame")        // Load in each frames skeleton (the parts of each joint that changed from the base frame)
            {
                FrameData tempFrame;

                fileIn >> tempFrame.frameID;        // Get the frame ID

                fileIn >> checkString;                // Skip opening bracket "{"

                for (int i = 0; i < tempAnim.numAnimatedComponents; i++)
                {
                    float tempData;
                    fileIn >> tempData;                // Get the data

                    tempFrame.frameData.push_back(tempData);
                }

                tempAnim.frameData.push_back(tempFrame);

                ///*** build the frame skeleton ***///
                std::vector<Joint> tempSkeleton;

                for (int i = 0; i < tempAnim.jointInfo.size(); i++)
                {
                    int k = 0;                        // Keep track of position in frameData array

                    // Start the frames joint with the base frame's joint
                    Joint tempFrameJoint = tempAnim.baseFrameJoints[i];

                    tempFrameJoint.parentID = tempAnim.jointInfo[i].parentID;

                    // Notice how I have been flipping y and z. this is because some modeling programs such as
                    // 3ds max (which is what I use) use a right handed coordinate system. Because of this, we
                    // need to flip the y and z axes. If your having problems loading some models, it's possible
                    // the model was created in a left hand coordinate system. in that case, just reflip all the
                    // y and z axes in our md5 mesh and anim loader.
                    if (tempAnim.jointInfo[i].flags & 1)        // pos.x    ( 000001 )
                        tempFrameJoint.pos.x = tempFrame.frameData[tempAnim.jointInfo[i].startIndex + k++];

                    if (tempAnim.jointInfo[i].flags & 2)        // pos.y    ( 000010 )
                        tempFrameJoint.pos.z = tempFrame.frameData[tempAnim.jointInfo[i].startIndex + k++];

                    if (tempAnim.jointInfo[i].flags & 4)        // pos.z    ( 000100 )
                        tempFrameJoint.pos.y = tempFrame.frameData[tempAnim.jointInfo[i].startIndex + k++];

                    if (tempAnim.jointInfo[i].flags & 8)        // orientation.x    ( 001000 )
                        tempFrameJoint.orientation.x = tempFrame.frameData[tempAnim.jointInfo[i].startIndex + k++];

                    if (tempAnim.jointInfo[i].flags & 16)    // orientation.y    ( 010000 )
                        tempFrameJoint.orientation.z = tempFrame.frameData[tempAnim.jointInfo[i].startIndex + k++];

                    if (tempAnim.jointInfo[i].flags & 32)    // orientation.z    ( 100000 )
                        tempFrameJoint.orientation.y = tempFrame.frameData[tempAnim.jointInfo[i].startIndex + k++];


                    // Compute the quaternions w
                    float t = 1.0f - (tempFrameJoint.orientation.x * tempFrameJoint.orientation.x)
                        - (tempFrameJoint.orientation.y * tempFrameJoint.orientation.y)
                        - (tempFrameJoint.orientation.z * tempFrameJoint.orientation.z);
                    if (t < 0.0f)
                    {
                        tempFrameJoint.orientation.w = 0.0f;
                    }
                    else
                    {
                        tempFrameJoint.orientation.w = -sqrtf(t);
                    }

                    // Now, if the upper arm of your skeleton moves, you need to also move the lower part of your arm, and then the hands, and then finally the fingers (possibly weapon or tool too)
                    // This is where joint hierarchy comes in. We start at the top of the hierarchy, and move down to each joints child, rotating and translating them based on their parents rotation
                    // and translation. We can assume that by the time we get to the child, the parent has already been rotated and transformed based of it's parent. We can assume this because
                    // the child should never come before the parent in the files we loaded in.
                    if (tempFrameJoint.parentID >= 0)
                    {
                        Joint parentJoint = tempSkeleton[tempFrameJoint.parentID];

                        // Turn the XMFLOAT3 and 4's into vectors for easier computation
                        XMVECTOR parentJointOrientation = XMVectorSet(parentJoint.orientation.x, parentJoint.orientation.y, parentJoint.orientation.z, parentJoint.orientation.w);
                        XMVECTOR tempJointPos = XMVectorSet(tempFrameJoint.pos.x, tempFrameJoint.pos.y, tempFrameJoint.pos.z, 0.0f);
                        XMVECTOR parentOrientationConjugate = XMVectorSet(-parentJoint.orientation.x, -parentJoint.orientation.y, -parentJoint.orientation.z, parentJoint.orientation.w);

                        // Calculate current joints position relative to its parents position
                        XMFLOAT3 rotatedPos;
                        XMStoreFloat3(&rotatedPos, XMQuaternionMultiply(XMQuaternionMultiply(parentJointOrientation, tempJointPos), parentOrientationConjugate));

                        // Translate the joint to model space by adding the parent joint's pos to it
                        tempFrameJoint.pos.x = rotatedPos.x + parentJoint.pos.x;
                        tempFrameJoint.pos.y = rotatedPos.y + parentJoint.pos.y;
                        tempFrameJoint.pos.z = rotatedPos.z + parentJoint.pos.z;

                        // Currently the joint is oriented in its parent joints space, we now need to orient it in
                        // model space by multiplying the two orientations together (parentOrientation * childOrientation) <- In that order
                        XMVECTOR tempJointOrient = XMVectorSet(tempFrameJoint.orientation.x, tempFrameJoint.orientation.y, tempFrameJoint.orientation.z, tempFrameJoint.orientation.w);
                        tempJointOrient = XMQuaternionMultiply(parentJointOrientation, tempJointOrient);

                        // Normalize the orienation quaternion
                        tempJointOrient = XMQuaternionNormalize(tempJointOrient);

                        XMStoreFloat4(&tempFrameJoint.orientation, tempJointOrient);
                    }

                    // Store the joint into our temporary frame skeleton
                    tempSkeleton.push_back(tempFrameJoint);
                }

                // Push back our newly created frame skeleton into the animation's frameSkeleton array
                tempAnim.frameSkeleton.push_back(tempSkeleton);

                fileIn >> checkString;                // Skip closing bracket "}"
            }
        }

        // Calculate and store some usefull animation data
        tempAnim.frameTime = 1.0f / tempAnim.frameRate;                        // Set the time per frame
        tempAnim.totalAnimTime = tempAnim.numFrames * tempAnim.frameTime;    // Set the total time the animation takes
        tempAnim.currAnimTime = 0.0f;                                        // Set the current time to zero

        m_model.animations.push_back(tempAnim);                            // Push back the animation into our model object
    }
    else    // If the file was not loaded
    {
        // create message
        std::wstring message = L"Could not open: ";
        message += filename;

        MessageBox(0, message.c_str(),                // display message
            L"Error", MB_OK);

        return false;
    }
    return true;
}

void MD5Model::UpdateMD5Model(float dt, int animation)
{
    m_model.animations[animation].currAnimTime += dt;            // Update the current animation time

    if (m_model.animations[animation].currAnimTime > m_model.animations[animation].totalAnimTime)
        m_model.animations[animation].currAnimTime = 0.0f;

    // Which frame are we on
    float currentFrame = m_model.animations[animation].currAnimTime * m_model.animations[animation].frameRate;
    int frame0 = floorf(currentFrame);
    int frame1 = frame0 + 1;

    // Make sure we don't go over the number of frames    
    if (frame0 == m_model.animations[animation].numFrames - 1)
        frame1 = 0;

    float interpolation = currentFrame - frame0;    // Get the remainder (in time) between frame0 and frame1 to use as interpolation factor

    std::vector<Joint> interpolatedSkeleton;        // Create a frame skeleton to store the interpolated skeletons in

    // Compute the interpolated skeleton
    for (int i = 0; i < m_model.animations[animation].numJoints; i++)
    {
        Joint tempJoint;
        Joint joint0 = m_model.animations[animation].frameSkeleton[frame0][i];        // Get the i'th joint of frame0's skeleton
        Joint joint1 = m_model.animations[animation].frameSkeleton[frame1][i];        // Get the i'th joint of frame1's skeleton

        tempJoint.parentID = joint0.parentID;                                            // Set the tempJoints parent id

        // Turn the two quaternions into XMVECTORs for easy computations
        XMVECTOR joint0Orient = XMVectorSet(joint0.orientation.x, joint0.orientation.y, joint0.orientation.z, joint0.orientation.w);
        XMVECTOR joint1Orient = XMVectorSet(joint1.orientation.x, joint1.orientation.y, joint1.orientation.z, joint1.orientation.w);

        // Interpolate positions
        tempJoint.pos.x = joint0.pos.x + (interpolation * (joint1.pos.x - joint0.pos.x));
        tempJoint.pos.y = joint0.pos.y + (interpolation * (joint1.pos.y - joint0.pos.y));
        tempJoint.pos.z = joint0.pos.z + (interpolation * (joint1.pos.z - joint0.pos.z));

        // Interpolate orientations using spherical interpolation (Slerp)
        XMStoreFloat4(&tempJoint.orientation, XMQuaternionSlerp(joint0Orient, joint1Orient, interpolation));

        interpolatedSkeleton.push_back(tempJoint);        // Push the joint back into our interpolated skeleton
    }

    for (int k = 0; k < m_model.numSubsets; k++)
    {
        for (int i = 0; i < m_model.subsets[k].vertices.size(); ++i)
        {
            Vertex tempVert = m_model.subsets[k].vertices[i];
            tempVert.pos = XMFLOAT3(0, 0, 0);    // Make sure the vertex's pos is cleared first
            tempVert.normal = XMFLOAT3(0, 0, 0);    // Clear vertices normal

            // Sum up the joints and weights information to get vertex's position and normal
            for (int j = 0; j < tempVert.WeightCount; ++j)
            {
                Weight tempWeight = m_model.subsets[k].weights[tempVert.StartWeight + j];
                Joint tempJoint = interpolatedSkeleton[tempWeight.jointID];

                // Convert joint orientation and weight pos to vectors for easier computation
                XMVECTOR tempJointOrientation = XMVectorSet(tempJoint.orientation.x, tempJoint.orientation.y, tempJoint.orientation.z, tempJoint.orientation.w);
                XMVECTOR tempWeightPos = XMVectorSet(tempWeight.pos.x, tempWeight.pos.y, tempWeight.pos.z, 0.0f);

                // We will need to use the conjugate of the joint orientation quaternion
                XMVECTOR tempJointOrientationConjugate = XMQuaternionInverse(tempJointOrientation);

                // Calculate vertex position (in joint space, eg. rotate the point around (0,0,0)) for this weight using the joint orientation quaternion and its conjugate
                // We can rotate a point using a quaternion with the equation "rotatedPoint = quaternion * point * quaternionConjugate"
                XMFLOAT3 rotatedPoint;
                XMStoreFloat3(&rotatedPoint, XMQuaternionMultiply(XMQuaternionMultiply(tempJointOrientation, tempWeightPos), tempJointOrientationConjugate));

                // Now move the verices position from joint space (0,0,0) to the joints position in world space, taking the weights bias into account
                tempVert.pos.x += (tempJoint.pos.x + rotatedPoint.x) * tempWeight.bias;
                tempVert.pos.y += (tempJoint.pos.y + rotatedPoint.y) * tempWeight.bias;
                tempVert.pos.z += (tempJoint.pos.z + rotatedPoint.z) * tempWeight.bias;

                // Compute the normals for this frames skeleton using the weight normals from before
                // We can comput the normals the same way we compute the vertices position, only we don't have to translate them (just rotate)
                XMVECTOR tempWeightNormal = XMVectorSet(tempWeight.normal.x, tempWeight.normal.y, tempWeight.normal.z, 0.0f);

                // Rotate the normal
                XMStoreFloat3(&rotatedPoint, XMQuaternionMultiply(XMQuaternionMultiply(tempJointOrientation, tempWeightNormal), tempJointOrientationConjugate));

                // Add to vertices normal and ake weight bias into account
                tempVert.normal.x -= rotatedPoint.x * tempWeight.bias;
                tempVert.normal.y -= rotatedPoint.y * tempWeight.bias;
                tempVert.normal.z -= rotatedPoint.z * tempWeight.bias;
            }

            m_model.subsets[k].positions[i] = tempVert.pos;                // Store the vertices position in the position vector instead of straight into the vertex vector
            m_model.subsets[k].vertices[i].normal = tempVert.normal;        // Store the vertices normal
            XMStoreFloat3(&m_model.subsets[k].vertices[i].normal, XMVector3Normalize(XMLoadFloat3(&m_model.subsets[k].vertices[i].normal)));
        }

        // Put the positions into the vertices for this subset
        for (int i = 0; i < m_model.subsets[k].vertices.size(); i++)
        {
            m_model.subsets[k].vertices[i].pos = m_model.subsets[k].positions[i];
        }

        ///     
        CD3DX12_RANGE readRange(0, 0);
        void* data = nullptr;
        m_model.subsets[k].m_vBufferUploadHeap->Map(0, &readRange, &data);

        memcpy(data, m_model.subsets[k].vertices.data(), sizeof(Vertex) * m_model.subsets[k].vertices.size());

        m_model.subsets[k].m_vBufferUploadHeap->Unmap(0, nullptr);

        // Update the subsets vertex buffer
        // First lock the buffer
        //D3D11_MAPPED_SUBRESOURCE mappedVertBuff;
        //hr = d3d11DevCon->Map(m_model.subsets[k].vertBuff, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedVertBuff);

        //// Copy the data into the vertex buffer.
        //memcpy(mappedVertBuff.pData, &m_model.subsets[k].vertices[0], (sizeof(Vertex) * m_model.subsets[k].vertices.size()));

        //d3d11DevCon->Unmap(m_model.subsets[k].vertBuff, 0);

        // The line below is another way to update a buffer. You will use this when you want to update a buffer less
        // than once per frame, since the GPU reads will be faster (the buffer was created as a DEFAULT buffer instead
        // of a DYNAMIC buffer), and the CPU writes will be slower. You can try both methods to find out which one is faster
        // for you. if you want to use the line below, you will have to create the buffer with D3D11_USAGE_DEFAULT instead
        // of D3D11_USAGE_DYNAMIC
        //d3d11DevCon->UpdateSubresource( MD5Model.subsets[k].vertBuff, 0, NULL, &MD5Model.subsets[k].vertices[0], 0, 0 );
    }
}

void MD5Model::ReleaseUploadHeaps()
{
    for (auto& i: m_model.subsets)
    {
        if (i.m_vBufferUploadHeap)
        {
            i.m_vBufferUploadHeap->Release();
            i.m_vBufferUploadHeap = nullptr;
        }
        if (i.m_iBufferUploadHeap)
        {
            i.m_iBufferUploadHeap->Release();
            i.m_iBufferUploadHeap = nullptr;
        }
    }
}

void MD5Model::CleanUp()
{
    for (int i = 0; i < m_model.numSubsets; i++)
    {
        m_model.subsets[i].m_indexBuffer->Release();
        m_model.subsets[i].m_vertexBuffer->Release();
    }
}

std::vector<MD5Model::ModelSubset>& MD5Model::GetModelSubsets()
{
    return m_model.subsets;
}

bool MD5Model::CreateVertexBuffers(ID3D12Device6* device, ID3D12GraphicsCommandList6* commandList, ModelSubset& subset)
{
    HRESULT hr;

    int nrOfVertices = subset.vertices.size();
    int vBufferSize = sizeof(MD5Model::Vertex) * nrOfVertices;

    // create default heap
    // default heap is memory on the GPU. Only the GPU has access to this memory
    // To get data into this heap, we will have to upload the data using
    // an upload heap
    device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
        D3D12_HEAP_FLAG_NONE, // no flags
        &CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
        D3D12_RESOURCE_STATE_COPY_DEST, // we will start this heap in the copy destination state since we will copy data
                                        // from the upload heap to this heap
        nullptr, // optimized clear value must be null for this type of resource. used for render targets and depth/stencil buffers
        IID_PPV_ARGS(&subset.m_vertexBuffer));

    // we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
    subset.m_vertexBuffer->SetName(L"Vertex Buffer Resource Heap");

    // create upload heap
    // upload heaps are used to upload data to the GPU. CPU can write to it, GPU can read from it
    // We will upload the vertex buffer using this heap to the default heap
    subset.m_vBufferUploadHeap = nullptr;
    device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
        D3D12_HEAP_FLAG_NONE, // no flags
        &CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
        D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
        nullptr,
        IID_PPV_ARGS(&subset.m_vBufferUploadHeap));
    subset.m_vBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

    // store vertex buffer in upload heap
    D3D12_SUBRESOURCE_DATA vertexData = {};
    vertexData.pData = reinterpret_cast<BYTE*>(subset.vertices.data()); // pointer to our vertex array
    vertexData.RowPitch = vBufferSize; // size of all our triangle vertex data
    vertexData.SlicePitch = vBufferSize; // also the size of our triangle vertex data

    // we are now creating a command with the command list to copy the data from
    // the upload heap to the default heap
    if (!subset.m_vBufferUploadHeap)
    {
        return false;
    }
    else
    {
        UpdateSubresources(commandList, subset.m_vertexBuffer, subset.m_vBufferUploadHeap, 0, 0, 1, &vertexData);
    }

    // transition the vertex buffer data from copy destination state to vertex buffer state
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(subset.m_vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));





    int nrOfIndices = subset.indices.size();
    int iBufferSize = sizeof(DWORD) * nrOfIndices;

    // create default heap to hold index buffer
    device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
        D3D12_HEAP_FLAG_NONE, // no flags
        &CD3DX12_RESOURCE_DESC::Buffer(iBufferSize), // resource description for a buffer
        D3D12_RESOURCE_STATE_COPY_DEST, // start in the copy destination state
        nullptr, // optimized clear value must be null for this type of resource
        IID_PPV_ARGS(& subset.m_indexBuffer));

    // create upload heap to upload index buffer
    
    device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
        D3D12_HEAP_FLAG_NONE, // no flags
        &CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), // resource description for a buffer
        D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
        nullptr,
        IID_PPV_ARGS(&subset.m_iBufferUploadHeap));
    subset.m_iBufferUploadHeap->SetName(L"Index Buffer Upload Resource Heap");

    // store vertex buffer in upload heap
    D3D12_SUBRESOURCE_DATA indexData = {};
    indexData.pData = reinterpret_cast<BYTE*>(subset.indices.data()); // pointer to our index array
    indexData.RowPitch = iBufferSize; // size of all our index buffer
    indexData.SlicePitch = iBufferSize; // also the size of our index buffer

    // we are now creating a command with the command list to copy the data from
    // the upload heap to the default heap
    if (!subset.m_iBufferUploadHeap)
    {
        return false;
    }
    else
    {
        UpdateSubresources(commandList, subset.m_indexBuffer, subset.m_iBufferUploadHeap, 0, 0, 1, &indexData);
    }

    // transition the vertex buffer data from copy destination state to vertex buffer state
    commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(subset.m_indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

    // create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
    //subset.m_vertexBufferView.BufferLocation = subset.m_vertexBuffer->GetGPUVirtualAddress();
    subset.m_vertexBufferView.BufferLocation = subset.m_vBufferUploadHeap->GetGPUVirtualAddress(); // temporärt upload heap för memcopy medans cpu animering
    subset.m_vertexBufferView.StrideInBytes = sizeof(MD5Model::Vertex);
    subset.m_vertexBufferView.SizeInBytes = vBufferSize;

    // create a vertex buffer view for the triangle.We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
    subset.m_indexBufferView.BufferLocation = subset.m_indexBuffer->GetGPUVirtualAddress();
    subset.m_indexBufferView.Format = DXGI_FORMAT_R32_UINT; // 32-bit unsigned integer (this is what a dword is, double word, a word is 2 bytes)
    subset.m_indexBufferView.SizeInBytes = iBufferSize;

    return true;
}

void MD5Model::UpdateVertexBuffer()
{
}
