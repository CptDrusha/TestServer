//
// Created by dml on 27.07.22.
//

struct ConversionSourceStruct
{
    ConversionSourceStruct() : PAID(-1), MF(0.f) {}

    ConversionSourceStruct(int paid, float mf)
        : PAID(paid), MF(mf) {}

    int PAID;
    float MF;
};

struct ConversionProductStruct
{
    ConversionProductStruct() : PAID(-1), PR(0.f), QM(0.f) {}

    ConversionProductStruct(int paid, float pr, float qm)
        : PAID(paid), PR(pr), QM(qm) {}

    int PAID;
    float PR;
    float QM;
};

struct ContainerPhantomCheckData
{
    ContainerPhantomCheckData(ContainerInstance container, float phM, float phV)
        : Container(container), phMass(phM), phVolume(phV) {}

    ContainerInstance Container;
    float phMass;
    float phVolume;
};