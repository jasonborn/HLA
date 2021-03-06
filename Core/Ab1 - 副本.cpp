#include "Ab1.h"
#include <QDebug>
#include <QSharedPointer>

#define MAX(a,b) (((a)>(b))?(a):(b))
#define F_THRESHOLD 0.3

Ab1::Ab1(const QString &str_path):m_file(str_path)
{
    m_file.open(QIODevice::ReadOnly);
}

Ab1::~Ab1()
{
    m_file.close();
}

bool Ab1::AnalysisFile()
{
    qint32 l_Offset = 0;

    m_file.seek(26);
    QDataStream data(&m_file);
    data>>l_Offset;
    qDebug() << "l_Offset:" << l_Offset;
    m_file.seek(l_Offset);

    AB1Tag tag_temp;
    do
    {
        memset(&tag_temp,0,sizeof(AB1Tag));
        data>>tag_temp.name>>tag_temp.number>>tag_temp.elementtype>>tag_temp.elementsize
            >>tag_temp.numelements>>tag_temp.datasize>>tag_temp.dataoffset>>tag_temp.datahandle;

        m_multiHash.insertMulti(tag_temp.name,tag_temp);
//        qDebug() <<tag_temp.name<<" "<<tag_temp.number<<" "<<tag_temp.elementtype<<" "<<tag_temp.elementsize<<" "
//                                 <<tag_temp.numelements<<" "<<tag_temp.datasize<<" "<<tag_temp.dataoffset ;
    }while(tag_temp.name!=0);

    if(m_multiHash.empty())
    {
        return false;
    }
    return true;
}

bool Ab1::IsAb1File()
{
    QDataStream data(&m_file);
    qint8 buf[5] = {0};
    data>>buf[0]>>buf[1]>>buf[2]>>buf[3]>>buf[4];

    if(0 != strcmp((char*)buf,"ABIF"))
    {
        return false;
    }
    return true;
}

int Ab1::getNumBase()
{
    long BASEtag = (long)(((((('P'<<8)+'B')<<8)+'A')<<8)+'S');

    QList<AB1Tag> value_list = m_multiHash.values(BASEtag);
    foreach (AB1Tag tag, value_list)
    {
        if(tag.number == 2)
        {
            return tag.numelements;
        }
    }
    return 0;
}

int Ab1::getBaseOffset()
{
    long BASEtag = ((long) ((((('P'<<8)+'B')<<8)+'A')<<8)+'S');
    QList<AB1Tag> value_list = m_multiHash.values(BASEtag);
    foreach (AB1Tag tag, value_list)
    {
        if(tag.number == 2)
        {
            return tag.dataoffset;
        }
    }
    return 0;
}

int Ab1::getBasePosOffset()
{
    long BASEtag = ((long) ((((('P'<<8)+'L')<<8)+'O')<<8)+'C');
    QList<AB1Tag> value_list = m_multiHash.values(BASEtag);
    foreach (AB1Tag tag, value_list)
    {
        if(tag.number == 2)
        {
            return tag.dataoffset;
        }
    }
    return 0;
}

int Ab1::getQualOffset()
{
    long BASEtag = ((long) ((((('P'<<8)+'C')<<8)+'O')<<8)+'N');
    QList<AB1Tag> value_list = m_multiHash.values(BASEtag);
    foreach (AB1Tag tag, value_list)
    {
        if(tag.number == 2)
        {
            return tag.dataoffset;
        }
    }
    return 0;
}

int Ab1::getNumTraceData()
{
    long BASEtag = ((long) ((((('D'<<8)+'A')<<8)+'T')<<8)+'A');
    QList<AB1Tag> value_list = m_multiHash.values(BASEtag);
    foreach (AB1Tag tag, value_list)
    {
        if(tag.number == 9)
        {
            return tag.numelements;
        }
    }
    return 0;
}

bool Ab1::getFWO()
{
    long BASEtag = ((long) ((((('F'<<8)+'W')<<8)+'O')<<8)+'_');
    QList<AB1Tag> value_list = m_multiHash.values(BASEtag);
    foreach (AB1Tag tag, value_list)
    {
        if(tag.number == 1)
        {
             m_cFWO = &tag.dataoffset;
        }
    }
    return true;
}

float IntToFloat(int num)
{
    float fValue;
    memcpy(&fValue, &num, sizeof(int));
    return fValue;
}

float Ab1::getBaseSpace()
{
    long BASEtag = ((long) ((((('S'<<8)+'P')<<8)+'A')<<8)+'C');
    QList<AB1Tag> value_list = m_multiHash.values(BASEtag);
    foreach (AB1Tag tag, value_list)
    {
        if(tag.number == 1)
        {
            return IntToFloat(tag.dataoffset);
        }
    }
    return 0;
}

bool Ab1::getTraceDataOffset(long *Offset)
{
    long BASEtag = ((long) ((((('D'<<8)+'A')<<8)+'T')<<8)+'A');
    QList<AB1Tag> value_list = m_multiHash.values(BASEtag);
    foreach (AB1Tag tag, value_list)
    {
        switch (tag.number) {
        case 9:
            Offset[0] = tag.dataoffset;
            break;
        case 10:
            Offset[1] = tag.dataoffset;
            break;
        case 11:
            Offset[2] = tag.dataoffset;
            break;
        case 12:
            Offset[3] = tag.dataoffset;
            break;
        default:
            break;
        }
    }
}

bool Ab1::isAb1Good()
{
     long BASEtag = ((long) ((((('S'<<8)+'/')<<8)+'N')<<8)+'%');
     QList<AB1Tag> value_list = m_multiHash.values(BASEtag);
     foreach (AB1Tag tag, value_list)
     {
         if(tag.number == 1)
         {
             m_file.seek(tag.dataoffset);
             QDataStream data(&m_file);
             qint16 sig1=sig2=sig3=sig4=0;
             data>>sig1>>sig2>>sig3>>sig4;
             long l_total = sig1 + sig2 + sig3 + sig4;
             if(l_total > 800)
             {
                 return true;
             }
         }
     }
     return false;
}

bool Ab1::getBaseSeq(long offset, long num, char* pseq)
{
    m_file.seek(offset);
    QDataStream data(&m_file);
    data.readRawData(pseq, num);
    return true;
}

bool Ab1::getBaseQual(long offset, long num, int *qualArry, int &maxQual)
{
    m_file.seek(offset);
    QDataStream data(&m_file);
    char *ptemp = new char[num];
    if(ptemp)
    {
        data.readRawData(pseq, num);
        maxqual = 0;
        for(int i=0;i<num;i++)
        {
            qualArry[i] = ptemp[i];
            if(qualArry[i] > maxQual)
            {
                maxQual = qualArry[i];//总是保存最大值
            }
        }
    }

    return true;
}

bool Ab1::getBasePosition(long offset, long num, int *posArry)
{
    m_file.seek(offset);
    QDataStream data(&m_file);
    for(int i=0;i<num;i++)
    {
        qint16 pos = 0;
        data>>pos;
        posArry[i] = pos;
    }
    return true;
}

bool Ab1::getTraceData(Baseo *baseG, Baseo *baseA, Baseo *baseT, Baseo *baseC,
                       long *offset, long num,  int &maxSignal)
{
    for (int i=0;i<4;i++)
    {
        m_file.seek(offset[i]);
        for (int j=0;j<num;j++)
        {
            QDataStream data(&m_file);
            qint16 signal = 0;
            data>>signal;

            if (m_cFWO[i] == 'G')
            {
                baseG[j].basetype = 'G';
                baseG[j].pos = j + 1;
                baseG[j].signal = signal;
                baseG[j].qual = 0;

                if (baseG[j].signal > maxSignal)
                {
                    maxSignal = baseG[j].signal;
                }
            }
            else if (m_cFWO[i] == 'A')
            {
                baseA[j].basetype = 'A';
                baseA[j].pos = j + 1;
                baseA[j].signal = signal;
                baseA[j].qual = 0;

                if (baseA[j].signal > maxSignal)
                {
                    maxSignal = baseA[j].signal;
                }
            }
            else if (m_cFWO[i] == 'T')
            {
                baseT[j].basetype = 'T';
                baseT[j].pos = j + 1;
                baseT[j].signal = signal;
                baseT[j].qual = 0;

                if (baseT[j].signal > maxSignal)
                {
                    maxSignal = baseT[j].signal;
                }
            }
            else if(m_cFWO[i] == 'C')
            {
                baseC[j].basetype = 'C';
                baseC[j].pos = j + 1;
                baseC[j].signal = signal;
                baseC[j].qual = 0;

                if (baseC[j].signal > maxSignal)
                {
                    maxSignal = baseC[j].signal;
                }
            }
        }
    }
    return true;
}

bool Ab1::getBaseData(Baseo* base, long num, int *basqQual, char *baseSeq, int *basePos,
                      Baseo* baseG, Baseo* baseA, Baseo* baseT, Baseo* baseC)
{
    for(int i=0; i<num; i++)
    {
        base[i].basetype = baseSeq[i];
        base[i].pos = basePos[i];
        base[i].qual = basqQual[i];

        if(seq[i] == 'G')
        {
            base[i].signal = baseG[basePos[i]].signal;
        }
        else if(seq[i] == 'A')
        {
            base[i].signal = baseA[basePos[i]].signal;
        }
        else if(seq[i] == 'T')
        {
            base[i].signal = baseT[basePos[i]].signal;
        }
        else if(seq[i] == 'C')
        {
            base[i].signal = baseC[basePos[i]].signal;
        }
        else
        {
            int g = baseG[basePos[i]].signal;
            int a = baseA[basePos[i]].signal;
            int t = baseT[basePos[i]].signal;
            int c = baseC[basePos[i]].signal;
            base[i].signal = MAX(MAX(MAX(g,a),t),c);
        }
    }

    return true;
}

bool compare(Baseo* a, Baseo* b)
{
    return a->signal > b->signal;
}

bool Ab1::hasPeak_test(Baseo* base, int low, int high, int &pos)//先找指定范围内最大值，然后和左右的值进行比较
{
    QVector<Baseo> base_vec;
    for (int i = low; i < high; i++)
    {
        base_vec.push_back(base[i]);
    }
    qsort(base_vec.begin(),base_vec.end(),compare);

    const Baseo maxbase = base_vec.at(0);
    for (int i = low; i < high; i++)
    {
        if(base[i].signal == maxbase.signal &&
           base[i-1].signal <= maxbase.signal &&
           base[i+1].signal < maxbase.signal)
        {
            pos = i;
            return true;
        }
    }
    return false;
}

bool Ab1::hasPeak(Baseo* base, int low, int high, int &pos) //在一定范围比较5个点，如果满足条件，说明存在峰值
{
    for (int i = low; i < high; i++)
    {
        if (base[i-2].signal < base[i-1].signal &&
            base[i-1].signal <= base[i].signal &&
            base[i+1].signal < base[i].signal &&
            base[i+2].signal < base[i+1].signal)
        {
            pos = i;
            return true;
        }
    }
    return false;
}

bool Ab1::IsValidPeak(Baseo* base, int low, int high,int basePos,int signal)
{
    int pos = 0;
    bool ret1 = hasPeak(base, low, high, pos);
    bool ret2 = abs(pos - basePos) <= 4 ? true : false;
    bool ret3 = 1.0 * base[pos].signal / signal >= F_THRESHOLD ? true : false;
    return ret1&&ret2&&ret3;
}

bool Ab1::getPeak(Baseo* baseG, Baseo* baseA, Baseo* baseT, Baseo* baseC, Baseo* base,
                  char *baseSeq, int *basePos, long num)
{
    int low, high;
    for (i = 1; i < numBase - 1; i++)
    {
        if (base[i].basetype == 'N')
        {
            continue;
        }
        else if (base[i].basetype != 'G' && base[i].basetype != 'A' && base[i].basetype != 'T' && base[i].basetype != 'C')
        {
        }
        else
        {
            int pos = 0;
            low = basePos[i] - int((basePos[i] - basePos[i-1])/2);
            high = basePos[i] + int ((basePos[i+1] - basePos[i])/2);
            if (base[i].basetype == 'G' && base[i].qual <= 40)
            {
//                if (hasPeak(BaseA, low, high, pos) &&
//                    abs(pos - basePos[i]) <= 4 &&
//                    1.0 * BaseA[pos].signal / base[i].signal >= F_THRESHOLD)
                if(IsValidPeak(BaseA, low, high, basePos[i], base[i].signal))
                {
                    base[i].basetype = 'R';
                }
                else if (hasPeak(BaseT, low, high, pos) &&
                         abs(pos - basePos[i]) <= 4 &&
                         1.0 * BaseT[pos].signal / base[i].signal >= F_THRESHOLD)
                {
                    base[i].basetype = 'K';
                }
                else if (hasPeak(BaseC, low, high, pos) &&
                         abs(pos - basePos[i]) <= 4 &&
                         1.0 * BaseC[pos].signal / base[i].signal >= F_THRESHOLD)
                {
                    base[i].basetype = 'S';
                }
            }
            else if (base[i].basetype == 'A' && base[i].qual <= 40)
            {
                if (hasPeak(BaseG, low, high, pos) &&
                        abs(pos - basePos[i]) <= 4 &&
                        1.0 * BaseG[pos].signal / base[i].signal >= F_THRESHOLD)
                {
                    base[i].basetype = 'R';
                }
                else if (hasPeak(BaseT, low, high, pos) &&
                         abs(pos - basePos[i]) <= 4 &&
                         1.0 * BaseT[pos].signal / base[i].signal >= F_THRESHOLD)
                {
                    base[i].basetype = 'W';
                }
                else if (hasPeak(BaseC, low, high, pos) &&
                         abs(pos - basePos[i]) <= 4 &&
                         1.0 * BaseC[pos].signal / base[i].signal >= F_THRESHOLD)
                {
                    base[i].basetype = 'M';
                }
            }
            else if (base[i].basetype == 'T' && base[i].qual <= 40)
            {
                if (hasPeak(BaseG, low, high, pos) &&
                        abs(pos - basePos[i]) <= 4 &&
                        1.0 * BaseG[pos].signal / base[i].signal >= F_THRESHOLD)
                {
                    base[i].basetype = 'K';
                }
                else if (hasPeak(BaseA, low, high, pos) &&
                         abs(pos - basePos[i]) <= 4 &&
                         1.0 * BaseA[pos].signal / base[i].signal >= F_THRESHOLD)
                {
                    base[i].basetype = 'W';
                }
                else if (hasPeak(BaseC, low, high, pos) &&
                         abs(pos - basePos[i]) <= 4 &&
                         1.0 * BaseC[pos].signal / base[i].signal >= F_THRESHOLD)
                {
                    base[i].basetype = 'Y';
                }
            }
            else if (base[i].basetype == 'C' && base[i].qual <= 40)
            {
                if (hasPeak(BaseG, low, high, pos) &&
                        abs(pos - basePos[i]) <= 4 &&
                        1.0 * BaseG[pos].signal / base[i].signal >= F_THRESHOLD)
                {
                    base[i].basetype = 'S';
                }
                else if (hasPeak(BaseA, low, high, pos) &&
                         abs(pos - basePos[i]) <= 4 &&
                         1.0 * BaseA[pos].signal / base[i].signal >= F_THRESHOLD)
                {
                    base[i].basetype = 'M';
                }
                else if (hasPeak(BaseT, low, high, pos) &&
                         abs(pos - basePos[i]) <= 4 &&
                         1.0 * BaseT[pos].signal / base[i].signal >= F_THRESHOLD)
                {
                    base[i].basetype = 'Y';
                }
            }
        }
    }
    return true;
}


