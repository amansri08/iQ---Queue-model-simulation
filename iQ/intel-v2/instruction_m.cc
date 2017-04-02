//
// Generated file, do not edit! Created by opp_msgc 4.3 from instruction.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "instruction_m.h"

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
std::ostream& operator<<(std::ostream& out,const T&) {return out;}

// Another default rule (prevents compiler from choosing base class' doPacking())
template<typename T>
void doPacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doPacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}

template<typename T>
void doUnpacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doUnpacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}



namespace BranchComp {

Register_Class(Instruction);

Instruction::Instruction(const char *name, int kind) : cMessage(name,kind)
{
    this->dep_var = 0;
    this->type_var = 0;
    this->id_var = 0;
}

Instruction::Instruction(const Instruction& other) : cMessage(other)
{
    copy(other);
}

Instruction::~Instruction()
{
}

Instruction& Instruction::operator=(const Instruction& other)
{
    if (this==&other) return *this;
    cMessage::operator=(other);
    copy(other);
    return *this;
}

void Instruction::copy(const Instruction& other)
{
    this->dep_var = other.dep_var;
    this->type_var = other.type_var;
    this->id_var = other.id_var;
}

void Instruction::parsimPack(cCommBuffer *b)
{
    cMessage::parsimPack(b);
    doPacking(b,this->dep_var);
    doPacking(b,this->type_var);
    doPacking(b,this->id_var);
}

void Instruction::parsimUnpack(cCommBuffer *b)
{
    cMessage::parsimUnpack(b);
    doUnpacking(b,this->dep_var);
    doUnpacking(b,this->type_var);
    doUnpacking(b,this->id_var);
}

long Instruction::getDep() const
{
    return dep_var;
}

void Instruction::setDep(long dep)
{
    this->dep_var = dep;
}

const char * Instruction::getType() const
{
    return type_var.c_str();
}

void Instruction::setType(const char * type)
{
    this->type_var = type;
}

long Instruction::getId() const
{
    return id_var;
}

void Instruction::setId(long id)
{
    this->id_var = id;
}

class InstructionDescriptor : public cClassDescriptor
{
  public:
    InstructionDescriptor();
    virtual ~InstructionDescriptor();

    virtual bool doesSupport(cObject *obj) const;
    virtual const char *getProperty(const char *propertyname) const;
    virtual int getFieldCount(void *object) const;
    virtual const char *getFieldName(void *object, int field) const;
    virtual int findField(void *object, const char *fieldName) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getArraySize(void *object, int field) const;

    virtual std::string getFieldAsString(void *object, int field, int i) const;
    virtual bool setFieldAsString(void *object, int field, int i, const char *value) const;

    virtual const char *getFieldStructName(void *object, int field) const;
    virtual void *getFieldStructPointer(void *object, int field, int i) const;
};

Register_ClassDescriptor(InstructionDescriptor);

InstructionDescriptor::InstructionDescriptor() : cClassDescriptor("BranchComp::Instruction", "cMessage")
{
}

InstructionDescriptor::~InstructionDescriptor()
{
}

bool InstructionDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<Instruction *>(obj)!=NULL;
}

const char *InstructionDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int InstructionDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 3+basedesc->getFieldCount(object) : 3;
}

unsigned int InstructionDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
    };
    return (field>=0 && field<3) ? fieldTypeFlags[field] : 0;
}

const char *InstructionDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "dep",
        "type",
        "id",
    };
    return (field>=0 && field<3) ? fieldNames[field] : NULL;
}

int InstructionDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='d' && strcmp(fieldName, "dep")==0) return base+0;
    if (fieldName[0]=='t' && strcmp(fieldName, "type")==0) return base+1;
    if (fieldName[0]=='i' && strcmp(fieldName, "id")==0) return base+2;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *InstructionDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "long",
        "string",
        "long",
    };
    return (field>=0 && field<3) ? fieldTypeStrings[field] : NULL;
}

const char *InstructionDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    }
}

int InstructionDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    Instruction *pp = (Instruction *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string InstructionDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    Instruction *pp = (Instruction *)object; (void)pp;
    switch (field) {
        case 0: return long2string(pp->getDep());
        case 1: return oppstring2string(pp->getType());
        case 2: return long2string(pp->getId());
        default: return "";
    }
}

bool InstructionDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    Instruction *pp = (Instruction *)object; (void)pp;
    switch (field) {
        case 0: pp->setDep(string2long(value)); return true;
        case 1: pp->setType((value)); return true;
        case 2: pp->setId(string2long(value)); return true;
        default: return false;
    }
}

const char *InstructionDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldStructNames[] = {
        NULL,
        NULL,
        NULL,
    };
    return (field>=0 && field<3) ? fieldStructNames[field] : NULL;
}

void *InstructionDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    Instruction *pp = (Instruction *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}

}; // end namespace BranchComp

