#include <bits/stdc++.h>
#include <cstdlib>
using namespace std;

#define ll long long
#define F first
#define S second
#define npos  string::npos
#define Hemz() (ios_base:: sync_with_stdio(false),cin.tie(NULL), cout.tie(NULL));

double totalWords;
set<int> available;
vector<map<string, int>> docs(4);
vector<vector<int>> vectors(4);
vector<vector<double>> smoothing(4);
vector<map<string, double>> prop(4);
vector<double> ans(4);
vector<bool> retrieved = {true, true, true, true};
vector<string> dictionary(370101), query = {"the", "fox"};

void readDictionary(ifstream &inputFile)
{
    string word;
    int i = 0;
    while (inputFile >> word)
    {
        dictionary[i] = word;
        i++;
    }
}

void generateRandomWords(ofstream &outputFile)
{
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(1, 370101);
    for (int i = 0; i < dis(gen) % 100; ++i)
    {
        string word = dictionary[dis(gen) % 370101];
        outputFile << word << " ";
        if (!i)
            query.emplace_back(word);
    }
    outputFile.close();
}

map<string, int> wordFreq(ifstream &inputFile)
{
    map<string,int> m;
    string word;

    while (inputFile >> word)
    {
        transform(word.begin(), word.end(), word.begin(), ::tolower);
        m[word]++;
        totalWords++;
    }

    inputFile.close();
    return m;
}

void createVector(vector<int> &v1, vector<int> &v2, set<string> &s, map<string, int> &doc)
{
    for (const string& word : s)
    {
        if (doc.count(word))
            v1.push_back(1);
        else
            v1.push_back(0);

        if (find(query.begin(), query.end(),word) != query.end())
            v2.push_back(1);
        else
            v2.push_back(0);
    }
}

double cosineSimilarity(vector<int> &v1,  vector<int> &v2)
{
    double similarity, dotProduct = 0.0, magDoc1 = 0.0, magDoc2 = 0.0;

    for (int i = 0; i < v1.size(); ++i)
    {
        dotProduct += v1[i] * v2[i];
        magDoc1 += pow(v1[i], 2);
        magDoc2 += pow(v2[i], 2);
    }

    similarity = dotProduct / (sqrt(magDoc1) * sqrt(magDoc2));

    return similarity;
}

void printAns()
{
    cout << "Document 1 = " << ans[0] << endl;
    cout << "Document 2 = " << ans[1] << endl;
    cout << "Document 3 = " << ans[2] << endl;
    cout << "Document 4 = " << ans[3] << endl;
}

void getCosineSimilarity()
{

    for (int i = 0; i < 4; ++i)
    {
        set<string> temp;
        vector<int> v;
        for (auto& item : query)
        {
            temp.insert(item);
        }
        for (auto& item : docs[i])
        {
            temp.insert(item.F);
        }
        createVector(vectors[i], v, temp, docs[i]);
        ans[i] = cosineSimilarity(v, vectors[i]) * 100;
    }

    cout << "================= Cosine Similarity ==================" << endl;
    printAns();
}

void evaluate(const vector<bool>& relevant)
{
    int number_of_relevant_and_retreved = 0;
    int number_of_retreved = 0;
    int number_of_relevant = 0;
    int sum_of_relevent_and_retreved = 0;
    vector <int> v(0,0);

    // Calculate True Positives, False Positives, and False Negatives
    for (size_t i = 0; i < relevant.size(); ++i)
    {
        if(relevant[i]){
            number_of_relevant++;

        }
        if(retrieved[i])
            number_of_retreved++;
        if(relevant[i] && retrieved[i]) {
            number_of_relevant_and_retreved++;
            v.push_back(number_of_retreved);
        }

    }

    // Calculate Precision
    double precision = 0.0;
    if (number_of_retreved > 0) {
        precision = static_cast<double>(number_of_relevant_and_retreved) / (number_of_retreved);
    }

    // Calculate Recall
    double recall = 0.0;
    if (number_of_relevant > 0) {
        recall = static_cast<double>(number_of_relevant_and_retreved) / (number_of_relevant);
    }

    // Calculate F-measure
    double f_measure = 0.0;
    if (precision + recall > 0) {
        f_measure = (2 * precision * recall) / (precision + recall);
    }

    // Calculate Rank Power
    double rank_power = 0 ;
    if(number_of_relevant_and_retreved>0){
        for(int x : v){
            sum_of_relevent_and_retreved += x;
        }
        rank_power = static_cast<double> (sum_of_relevent_and_retreved) / pow(number_of_relevant_and_retreved,2) ;
    }

    // Output results
    cout << "Precision: " << precision << endl;
    cout << "Recall: " << recall << endl;
    cout << "F-measure: " << f_measure << endl;
    cout << "Rank Power: " << rank_power << endl;
}

void getJaccard()
{

    for (int i = 0; i < 4; ++i)
    {
        set<string> queryUnique, documentUnique, result;

        for (auto &item : query)
        {
            queryUnique.insert(item);
        }
        for (auto& item : docs[i])
        {
            documentUnique.insert(item.F);
        }

        set_intersection(queryUnique.begin(), queryUnique.end(), documentUnique.begin(),
                         documentUnique.end(),
                         inserter(result, result.begin()));

        double intersection = result.size();

        set_union(queryUnique.begin(), queryUnique.end(), documentUnique.begin(),
                  documentUnique.end(),
                  inserter(result, result.begin()));

        double uniuon = result.size();

        ans[i] = intersection / uniuon;
    }

    cout << "================= Jaccard ==================" << endl;
    printAns();
}

void evaluateJelinekMercerSmoothing()
{
    vector<pair<double, int>> ranking(4);
    for (int i = 0; i < 4; ++i)
    {
        double evalute = 1;
        for (double num : smoothing[i])
            evalute *= num;

        ranking[i] = make_pair(evalute, i);
    }
    sort(ranking.begin(), ranking.end());

    for (int i = 3; i >= 0; --i)
    {
        printf("Document %i: %.2f\n", ranking[i].S + 1, ranking[i].F);
    }
}

void evaluateLikelihoodModel()
{
    vector<pair<double, string>> ranking;

    for (auto item : query)
    {
        double evalute = 1;
        for (auto doc : prop)
            evalute *= doc[item];

        ranking.push_back(make_pair(evalute, item));
    }

    sort(ranking.begin(), ranking.end());
    for (auto item: ranking)
    {
        printf("%s = %.4f\n", item.S.c_str(), item.F);
    }
}

void getJelinekMercerSmoothing()
{
    double lambda = 0.5;
    for (int i = 0; i < 4; ++i)
    {
        for (auto item : query)
        {
            double words = 0, word = 0, alldocs = 0;
            if (docs[i].count(item))
                word = docs[i][item];

            if (docs[0].count(item))
                alldocs += docs[i][item];
            if (docs[1].count(item))
                alldocs += docs[i][item];
            if (docs[2].count(item))
                alldocs += docs[i][item];
            if (docs[3].count(item))
                alldocs += docs[i][item];

            for (auto p : docs[i])
                words += p.second;

            double rule = (1 - lambda) * (word / words) + lambda * (alldocs/ totalWords);
            smoothing[i].push_back(rule);
        }
    }

    cout << "================= Jelinek Mercer Smoothing ==================" << endl;
    evaluateJelinekMercerSmoothing();
}

void getLikelihoodModel()
{
    for (int i = 0; i < 4; ++i)
    {
        for (auto item : query)
        {
            double words = 0, word = 0;
            if (docs[i].count(item))
                word = docs[i][item];

            for (auto p : docs[i])
                words += p.S;

            double rule = word/ words;
            prop[i][item] = rule;
        }
    }

    cout << "================= Likelihood Model ==================" << endl;
    evaluateLikelihoodModel();
}

vector<bool> convertVector()
{
    vector<bool> v(4, 0);
    for (auto item : available)
    {
        v[item - 1] = true;
    }

    return v;
}

void printReleventDocuments()
{
    cout << "Relevant Documents: \n";
    if (available.size() == 0)
        cout << "No Relevant Documents" << endl;
    for (auto item : available)
        cout << "Document " << item << endl;
}

void getBooleanSearch()
{
    cout << "Enter the Query: " << endl;
    available.clear();
    string parse;
    getline(cin, parse);

    vector<string> searchQuery;
    transform(parse.begin(), parse.end(), parse.begin(), ::tolower);

    while (!parse.empty())
    {
        auto find = parse.find(' ');

        if (find == npos)
            break;

        searchQuery.emplace_back(parse.substr(0, find));
        parse.erase(0, find +1);
    }
    searchQuery.emplace_back(parse);

    vector<string> search(searchQuery.size());

    int i = 0;
    for (auto item : searchQuery)
    {
        search[i] = item;
        i++;
    }

    for (int i = 0; i < search.size(); ++i)
   {
        if (search[i] == "or")
        {
            for (int j = 0; j < 4; ++j)
            {
                if (docs[j].count(search[i - 1]) || docs[j].count(search[i + 1]))
                {
                    available.insert(j+1);
                }
            }
        }
        else if (search[i] == "and" && search[i+1] == "not")
        {
            for (int j = 0; j < 4; ++j)
            {
                if (docs[j].count(search[i - 1]) && !docs[j].count(search[i + 2]))
                {
                    available.insert(j + 1);
                }
            }
        }
        else if (search[i] == "or" && search[i+1] == "not")
        {
            for (int j = 0; j < 4; ++j)
            {
                if (docs[j].count(search[i - 1]) || !docs[j].count(search[i + 2]))
                {
                    available.insert(j + 1);
                }
            }
        }
        else if (search[i] == "and")
        {
            for (int j = 0; j < 4; ++j)
            {
                if (docs[j].count(search[i - 1]) && docs[j].count(search[i + 1]))
                {
                    available.insert(j + 1);
                }
            }
        }
    }

    cout << "================= Boolean Search ==================" << endl;
    printReleventDocuments();
    cout << "================= Evaluation ==================" << endl;
    evaluate(convertVector());
}

void printStaticalRank()
{
    vector<pair<double, int>> ranking;

    for (int i = 0; i < 4; ++i)
    {
        double evalute = 0;
        for (auto item : query)
        {
            if (prop[i].count(item))
                evalute += prop[i][item];
        }
        ranking.push_back(make_pair(evalute,i+1));
    }

    sort(ranking.begin(), ranking.end(), greater<>());

    for (auto item : ranking)
    {
        printf("Document %i\n", item.S);
    }
}

void getStaticalModel()
{
    for (int i = 0; i < 4; ++i)
    {
        for (auto item : query)
        {
            double words = 0, word = 0;
            if (docs[i].count(item))
                word = docs[i][item];

            for (auto p : docs[i])
                words += p.S;

            double rule = word/ words;
            prop[i][item] = rule;
        }
    }
    cout << "================= Statical Model ==================" << endl;
    printStaticalRank();
}

int main()
{
    Hemz()

    string pathFile1 = "C:/Users/ibrah/CLionProjects/IR-Final-Project/doc1.txt";
    string pathFile2 = "C:/Users/ibrah/CLionProjects/IR-Final-Project/doc2.txt";
    string pathFile3 = "C:/Users/ibrah/CLionProjects/IR-Final-Project/doc3.txt";
    string pathFile4 = "C:/Users/ibrah/CLionProjects/IR-Final-Project/doc4.txt";
    string dict = "C:/Users/ibrah/CLionProjects/IR-Final-Project/dictionary.txt";

    ifstream inputFile1 (pathFile1, ios_base::in);
    ifstream inputFile2 (pathFile2, ios_base::in);
    ifstream inputFile3 (pathFile3, ios_base::in);
    ifstream inputFile4 (pathFile4, ios_base::in);
    ifstream dictFile (dict, ios_base::in);

//    ofstream outputFile1 (pathFile1, ios_base::out);
//    ofstream outputFile2 (pathFile2, ios_base::out);
//    ofstream outputFile3 (pathFile3, ios_base::out);
//    ofstream outputFile4 (pathFile4, ios_base::out);

//    readDictionary(dictFile);

//    generateRandomWords(outputFile1);
//    generateRandomWords(outputFile2);
//    generateRandomWords(outputFile3);
//    generateRandomWords(outputFile4);

    docs[0] = wordFreq(inputFile1);
    docs[1] = wordFreq(inputFile2);
    docs[2] = wordFreq(inputFile3);
    docs[3] = wordFreq(inputFile4);

    getCosineSimilarity();
    getJaccard();
    getJelinekMercerSmoothing();
    getLikelihoodModel();
    getBooleanSearch();
    getStaticalModel();

    return 0;
}