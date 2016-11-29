
#include "Classifier.h"


Classifier::Classifier()
{

}

Classifier::~Classifier()
{
    // Destory trees
    for (TreeNode* root : rootVec) treeBuilder.DestroyNode( root );
    rootVec.clear();
}


void Classifier::Train(
    const vector<Item>& iv, 
    const vector<NumericAttr>& fv, 
    const vector<char*>& cv )
{
    classVec = cv;
    featureVec = fv;

    // Randomly select features and build trees.
    unsigned int numFeatures = fv.size();

    printf( "Num features: %d\n", numFeatures );
    
    /********************** Use random sampler *********************/
    
    /*unsigned int* randomIndices = 
        (unsigned int*) malloc( numFeatures * sizeof( unsigned int ) );
    for (unsigned int i = 0; i < numFeatures; i++) randomIndices[i] = i;*/
    
    /**** Generate an ordered index container, and disorder it. ****/

    unsigned int* randomIndices = 
        (unsigned int*) malloc( numFeatures * sizeof( unsigned int ) );
    for (unsigned int i = 0; i < numFeatures; i++) randomIndices[i] = i;
    randomizeArray( randomIndices, numFeatures );

    /******************** Init tree constructer ********************/

    // Build a number of trees each having the same number of features.
    // What if numFeatures is 51 ?
    unsigned int numTrees = numFeatures / NUM_FEATURES_PER_TREE;
    rootVec.reserve( numTrees );
    treeBuilder.Init( fv, cv, NUM_FEATURES_PER_TREE );
    
    for (unsigned int treeIndex = 0; treeIndex < numTrees; treeIndex++)
    {
        /************** Use randomly disordered array **************/

        unsigned int* featureIndexArr = (unsigned int*) 
            malloc( NUM_FEATURES_PER_TREE * sizeof( unsigned int ) );
        memcpy( featureIndexArr, 
            randomIndices + treeIndex * NUM_FEATURES_PER_TREE, 
            NUM_FEATURES_PER_TREE * sizeof( unsigned int ) );
        
        /******************** Use random sampler *******************/
        /*unsigned int* featureIndexArr = 
            sampleWithRep( randomIndices, NUM_FEATURES_PER_TREE, numFeatures );*/

        treeBuilder.BuildTree( iv, featureIndexArr );
        rootVec.push_back( treeBuilder.GetRoot() );
    }

    free( randomIndices );
    randomIndices = nullptr;

    SaveModel();
}

void Classifier::Classify(
    const vector<Item>& iv, 
    const vector<char*>& cv )
{
    if (rootVec.empty())
    {
        LoadModel();
        //printf( "Please train the model first.\n" );
        //return;
    }

    unsigned int correctCounter = 0;
    unsigned int totalNumber = iv.size();

    printf( "Test set size: %u\n", totalNumber );

    classVec = cv;

    for (const Item& item : iv)
        if (Classify( item ) == item.classIndex) correctCounter++;

    float correctRate = (float) correctCounter / (float) totalNumber;
    float incorrectRate = 1.0f - correctRate;

    printf( "Correct rate: %f\n", correctRate );
    printf( "Incorrect rate: %f\n", incorrectRate );
}

int Classifier::Classify( const Item& item )
{
    unsigned short numClasses = classVec.size();
    unsigned int* votes = (unsigned int*) 
        calloc( numClasses, sizeof( unsigned int ) );

    for (const TreeNode* node : rootVec)
    {
        if (node == nullptr) continue;

        while (!node->childrenVec.empty())
        {
            unsigned int i = node->featureIndex;

            // 2 buckets by default:
            // one group having feature value smaller than threshold, 
            // another group having feature value greater than threshold.
            if (item.featureAttrArray[i] <= node->threshold)
            {
                if (node->childrenVec[0] == nullptr)
                    break;
                else
                    node = node->childrenVec[0];
            }
            else
            {
                if (node->childrenVec[1] == nullptr)
                    break;
                else
                    node = node->childrenVec[1];
            }
        }

        votes[node->classIndex]++;
    }

    unsigned short classIndex = getIndexOfMax( votes, numClasses );

    free( votes );
    votes = nullptr;

    return classIndex;
}

void Classifier::SaveModel()
{
    std::ofstream ofs( MODEL_FILE_PATH );
    
    if (ofs.good())
    {
        boost::archive::text_oarchive oa{ ofs };
        oa << rootVec;
    }
    else
    {
        printf( "Output stream is in an error state, saving model failed\n" );
    }

    ofs.flush();
    ofs.close();
}

void Classifier::LoadModel()
{
    if (!rootVec.empty())
    {
        printf( "Model loaded already!" );
        return;
    }

    std::ifstream ifs( MODEL_FILE_PATH );

    if (ifs.good())
    {
        boost::archive::text_iarchive ia{ ifs };
        ia >> rootVec;
    }
    else
    {
        printf( "Input stream is in an error state, loading model failed\n" );
    }

    ifs.close();
}