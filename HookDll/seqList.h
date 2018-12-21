class OutOfBound{};
class IllegalSize{};

template <class elemType>
class seqList
{
private:
    elemType* data;
    int currentLength;
    int maxSize;

    void doubleSpace();
public:
    seqList(int initSize = 8);
    ~seqList(){delete [] data;}
    void clear(){currentLength = 0;}
    int length() const{return currentLength;}
    void insert(int i, const elemType &x);
    void remove(int i);
    void append(const elemType &x);
    int search(const elemType &x)const;
    elemType visit(int i)const;
    void cut(int i);
    elemType* mydata();
};

template <class elemType>
void seqList <elemType>::doubleSpace()
{
    elemType* tmp = data;
    maxSize *= 2;
    data = new elemType[maxSize];
    for(int i = 0; i < currentLength; i++)data[i] = tmp[i];
    delete [] tmp;
}

template <class elemType>
seqList<elemType>::seqList(int initSize)
{
    if(initSize <= 0)throw IllegalSize();
    data = new elemType[initSize];
    maxSize = initSize;
    currentLength = 0;
}

template <class elemType>
void seqList<elemType>::insert(int i, const elemType &x)
{
    if(i < 0 || i > currentLength)throw OutOfBound();
    if(currentLength == maxSize)doubleSpace();
    for(int j = currentLength; j>i; j--)data[j] = data[j-1];
    data[i] = x;
    currentLength++;
}

template <class elemType>
void seqList<elemType>::remove(int i)
{
    if(i < 0 || i > currentLength - 1)throw OutOfBound();
    for(int j = i; j < currentLength - 1; j++)data[j] = data[j+1];
    currentLength--;
}

template <class elemType>
void seqList<elemType>::append(const elemType &x)
{
    if(currentLength == maxSize)doubleSpace();
    data[currentLength] = x;
    currentLength++;
}

template <class elemType>
int seqList<elemType>::search(const elemType &x) const
{
    int i;
    for(i = 0; i < currentLength && data[i] != x; i++);
    if(i == currentLength)return -1;
    else return i;
}

template <class elemType>
elemType seqList<elemType>::visit(int i)const
{
    if(i < 0 || i > currentLength - 1)throw OutOfBound();
    return data[i];
}

template <class elemType>
elemType* seqList <elemType>::mydata()
{
    return data;
}

template <class elemType>
void seqList<elemType>::cut(int i)
{
    if(i >= currentLength)return;
    currentLength = i;
}
