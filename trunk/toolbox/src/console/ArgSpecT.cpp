/* $Id: ArgSpecT.cpp,v 1.2 2001-12-14 19:51:01 paklein Exp $ */

#include "ArgSpecT.h"
#include <ctype.h>

/* array copy behavior */
const bool ArrayT<ArgSpecT*>::fByteCopy = true; 
const bool ArrayT<ArgSpecT>::fByteCopy = false; 

/* type names */
static const char* type_names[] = {
	"integer",
	"double",
	"string",
	"boolean",
	"float"};

/* constructor */
ArgSpecT::ArgSpecT(ArgTypeT t):
	fType(t),
	fDefault(NULL),
	fValue(NULL)
{

}

/* constructor */
ArgSpecT::ArgSpecT(ArgTypeT type, const StringT& name):
	fType(type),
	fName(name),
	fDefault(NULL),
	fValue(NULL)
{

}

/* copy constructor */
ArgSpecT::ArgSpecT(const ArgSpecT& arg):
	fDefault(NULL),
	fValue(NULL)
{
	operator=(arg);
}

/* destructor */
ArgSpecT::~ArgSpecT(void) { delete fDefault; }

/* clear the default value */
void ArgSpecT::ClearDefault(void)
{
	delete fDefault;
	fDefault = NULL;
}

/* return the type name */
const char* ArgSpecT::TypeName(void) const { return type_names[Type()]; }

/* clear the value */
void ArgSpecT::ClearValue(void)
{
	delete fValue;
	fValue = NULL;
}

/* set default value */
void ArgSpecT::SetDefault(const int& a)
{
	if (fType == int_) {
		delete fDefault;
		int* tmp = new int;
		*tmp = a;
		fDefault = tmp;
	}
	else if (fType == float_)
		SetDefault(float(a));
	else if (fType == double_)
		SetDefault(double(a));
	else {
		cout << "\n ArgSpecT::SetDefault: type mismatch" << endl;
		throw eGeneralFail;
	}
}

void ArgSpecT::SetDefault(const double& a)
{
	if (fType == double_) {
		delete fDefault;
		double* tmp = new double;
		*tmp = a;
		fDefault = tmp;
	}
	else {
		cout << "\n ArgSpecT::SetDefault: type mismatch" << endl;
		throw eGeneralFail;
	}	
}

void ArgSpecT::SetDefault(const StringT& a)
{
	if (fType == string_) {
		delete fDefault;
		StringT* tmp = new StringT;
		*tmp = a;
		fDefault = tmp;
	}
	else {
		cout << "\n ArgSpecT::SetDefault: type mismatch" << endl;
		throw eGeneralFail;
	}
}

void ArgSpecT::SetDefault(const bool& a)
{
	if (fType == bool_) {
		delete fDefault;
		bool* tmp = new bool;
		*tmp = a;
		fDefault = tmp;
	}
	else {
		cout << "\n ArgSpecT::SetDefault: type mismatch" << endl;
		throw eGeneralFail;
	}
}

void ArgSpecT::SetDefault(const float& a)
{
	if (fType == float_) {
		delete fDefault;
		float* tmp = new float;
		*tmp = a;
		fDefault = tmp;
	}
	else if (fType == double_)
		SetDefault(double(a));
	else {
		cout << "\n ArgSpecT::SetDefault: type mismatch" << endl;
		throw eGeneralFail;
	}
}

/* set default value */
void ArgSpecT::GetDefault(int& a) const
{
	if (fType == int_)
		a = *((int*) fDefault);
	else {
		cout << "\n ArgSpecT::GetDefault: type mismatch" << endl;
		throw eGeneralFail;
	}
}

void ArgSpecT::GetDefault(double& a) const
{
	if (fType == double_)
		a = *((double*) fDefault);
	else {
		cout << "\n ArgSpecT::GetDefault: type mismatch" << endl;
		throw eGeneralFail;
	}
}

void ArgSpecT::GetDefault(StringT& a) const
{
	if (fType == string_)
		a = *((StringT*) fDefault);
	else {
		cout << "\n ArgSpecT::GetDefault: type mismatch" << endl;
		throw eGeneralFail;
	}
}

void ArgSpecT::GetDefault(bool& a) const
{
	if (fType == bool_)
		a = *((bool*) fDefault);
	else {
		cout << "\n ArgSpecT::GetDefault: type mismatch" << endl;
		throw eGeneralFail;
	}
}

void ArgSpecT::GetDefault(float& a) const
{
	if (fType == float_)
		a = *((float*) fDefault);
	else {
		cout << "\n ArgSpecT::GetDefault: type mismatch" << endl;
		throw eGeneralFail;
	}
}

/* set the value to its default */
void ArgSpecT::SetValueToDefault(void)
{	
	/* set default */
	if (HasDefault())
	{
		switch (fType)
		{
			case int_:
			{
				int a;
				GetDefault(a);
				SetValue(a);
				break;
			}
			case double_:
			{
				double a;
				GetDefault(a);
				SetValue(a);
				break;
			}
			case string_:
			{
				StringT a;
				GetDefault(a);
				SetValue(a);
				break;
			}
			case bool_:
			{
				bool a;
				GetDefault(a);
				SetValue(a);
				break;
			}
			case float_:
			{
				float a;
				GetDefault(a);
				SetValue(a);
				break;
			}
			default:
				cout << "\n ArgSpecT::SetValueToDefault: unknown type: " 
				     << fType << endl;
				throw eGeneralFail;
		}	
	}	
}

/* assignment operator */
ArgSpecT& ArgSpecT::operator=(const ArgSpecT& rhs)
{
	fName = rhs.Name();
	fType = rhs.Type();
	fPrompt = rhs.Prompt();

	/* set default */
	if (rhs.HasDefault())
	{
		switch (fType)
		{
			case int_:
			{
				int a;
				rhs.GetDefault(a);
				SetDefault(a);
				break;
			}
			case double_:
			{
				double a;
				rhs.GetDefault(a);
				SetDefault(a);
				break;
			}
			case string_:
			{
				StringT a;
				rhs.GetDefault(a);
				SetDefault(a);
				break;
			}
			case bool_:
			{
				bool a;
				rhs.GetDefault(a);
				SetDefault(a);
				break;
			}
			case float_:
			{
				float a;
				rhs.GetDefault(a);
				SetDefault(a);
				break;
			}
			default:
				cout << "\n ArgSpecT::operator=: unknown default type" << endl;
				throw eGeneralFail;
		}	
	}	
	else
		ClearDefault();

	/* set value */
	if (rhs.HasValue())
	{
		switch (fType)
		{
			case int_:
			{
				int a;
				rhs.GetValue(a);
				SetValue(a);
				break;
			}
			case double_:
			{
				double a;
				rhs.GetValue(a);
				SetValue(a);
				break;
			}
			case string_:
			{
				StringT a;
				rhs.GetValue(a);
				SetValue(a);
				break;
			}
			case bool_:
			{
				bool a;
				rhs.GetValue(a);
				SetValue(a);
				break;
			}
			case float_:
			{
				float a;
				rhs.GetValue(a);
				SetValue(a);
				break;
			}
			default:
				cout << "\n ArgSpecT::operator=: unknown value type" << endl;
				throw eGeneralFail;
		}	
	}	
	else
		ClearValue();

	return *this;
}

/* read value from stream */
bool ArgSpecT::ReadValue(istream& in)
{
	switch (fType)
	{
		case int_:
		{
			int a = -991991;
			in >> a;
			if (a != -991991) {
				SetValue(a);
				return true;
			}
		}
		case double_:
		{
			double a = -991.991;
			in >> a;
			if (a != -991.991) {
				SetValue(a);
				return true;
			}
		}
		case string_:
		{
			StringT a;
			a.GetLineFromStream(in);
			if (a.StringLength() > 0) {
				SetValue(a);
				return true;
			}
		}
		case bool_:
		{
			/* get next non-white space character */
			char a;	
			in.get(a);
			while (in.good() && isspace(a)) 
				in.get(a);	

			/* resolve */
			switch (a)
			{
				case '1':
				case 't':
				case 'T':
					SetValue(true);
					return true;
				case '0':
				case 'f':
				case 'F':
					SetValue(false);
					return true;
				default:
					return false;
			}
		}
		case float_:
		{
			float a = -91.91;
			in >> a;
			if (a != -91.91) {
				SetValue(a);
				return true;
			}
		}
		default:
			cout << "\n ArgSpecT::operator=: unknown value type" << endl;
			throw eGeneralFail;
	}

	/* dummy */
	return false;
}

/* set default value */
void ArgSpecT::SetValue(const int& a)
{
	if (fType == int_) {
		delete fValue;
		int* tmp = new int;
		*tmp = a;
		fValue = (void*) tmp;
	}
	else if (fType == float_)
		SetValue(float(a));
	else if (fType == double_)
		SetValue(double(a));
	else {
		cout << "\n ArgSpecT::SetValue: type mismatch" << endl;
		throw eGeneralFail;
	}
}

void ArgSpecT::SetValue(const double& a)
{
	if (fType == double_) {
		delete fValue;
		double* tmp = new double;
		*tmp = a;
		fValue = (void*) tmp;
	}
	else {
		cout << "\n ArgSpecT::SetValue: type mismatch" << endl;
		throw eGeneralFail;
	}	
}

void ArgSpecT::SetValue(const StringT& a)
{
	if (fType == string_) {
		delete fValue;
		StringT* tmp = new StringT;
		*tmp = a;
		fValue = (void*) tmp;
	}
	else {
		cout << "\n ArgSpecT::SetValue: type mismatch" << endl;
		throw eGeneralFail;
	}
}

void ArgSpecT::SetValue(const bool& a)
{
	if (fType == bool_) {
		delete fValue;
		bool* tmp = new bool;
		*tmp = a;
		fValue = (void*) tmp;
	}
	else {
		cout << "\n ArgSpecT::SetValue: type mismatch" << endl;
		throw eGeneralFail;
	}
}

void ArgSpecT::SetValue(const float& a)
{
	if (fType == float_) {
		delete fValue;
		float* tmp = new float;
		*tmp = a;
		fValue = (void*) tmp;
	}
	else if (fType == double_)
		SetValue(double(a));
	else {
		cout << "\n ArgSpecT::SetValue: type mismatch" << endl;
		throw eGeneralFail;
	}
}

/* set default value */
void ArgSpecT::GetValue(int& a) const
{
	if (fType == int_)
		a = *((int*) fValue);
	else {
		cout << "\n ArgSpecT::GetValue: type mismatch" << endl;
		throw eGeneralFail;
	}
}

void ArgSpecT::GetValue(double& a) const
{
	if (fType == double_)
		a = *((double*) fValue);
	else {
		cout << "\n ArgSpecT::GetValue: type mismatch" << endl;
		throw eGeneralFail;
	}
}

void ArgSpecT::GetValue(StringT& a) const
{
	if (fType == string_)
		a = *((StringT*) fValue);
	else {
		cout << "\n ArgSpecT::GetValue: type mismatch" << endl;
		throw eGeneralFail;
	}
}

void ArgSpecT::GetValue(bool& a) const
{
	if (fType == bool_)
		a = *((bool*) fValue);
	else {
		cout << "\n ArgSpecT::GetValue: type mismatch" << endl;
		throw eGeneralFail;
	}
}

void ArgSpecT::GetValue(float& a) const
{
	if (fType == float_)
		a = *((float*) fValue);
	else {
		cout << "\n ArgSpecT::GetValue: type mismatch" << endl;
		throw eGeneralFail;
	}
}

/* write spec to output */
void ArgSpecT::Write(ostream& out) const
{
	out << '\t';
		
	/* name */
	if (Name().StringLength() > 0) out << Name() << " ";

	/* type */
	out << "(" << TypeName() << ")";

	/* current value */		
	if (HasValue())
	{
		out << " = ";
		WriteValue(out);
	}
	
	/* default value */
	if (HasDefault())
	{
		out << " (default = ";
		switch (fType)
		{
			case int_:
			{
				int a;
				GetDefault(a);
				out << a;
				break;
			}
			case double_:
			{
				double a;
				GetDefault(a);
				out << a;
				break;
			}
			case string_:
			{
				StringT a;
				GetDefault(a);
				out << '\"' << a << '\"';
				break;
			}
			case bool_:
			{
				bool a;
				GetDefault(a);
				if (a)
					out << "true";
				else
					out << "false";
				break;
			}
			case float_:
			{
				float a;
				GetDefault(a);
				out << a;
				break;
			}
			default:
				cout << "\n ArgSpecT::Write: unknown type: " << fType << endl;
				throw eGeneralFail;	
		}
		out << ')';
	}
	else
		out << " (no default)";
		
	/* prompt */
	if (Prompt().StringLength() > 0) out << " : " << Prompt();
}

/* write value to the output stream */
void ArgSpecT::WriteValue(ostream& out) const
{
	/* no value */
	if (!HasValue()) 
		out << "<unset>";
	else
	{
		switch (fType)
		{
			case int_:
			{
				int a;
				GetValue(a);
				out << a;
				break;
			}
			case double_:
			{
				double a;
				GetValue(a);
				out << a;
				break;
			}
			case string_:
			{
				StringT a;
				GetValue(a);
				out << '\"' << a << '\"';
				break;
			}
			case bool_:
			{
				bool a;
				GetValue(a);
				if (a)
					out << "true";
				else
					out << "false";
				break;
			}
			case float_:
			{
				float a;
				GetValue(a);
				out << a;
				break;
			}
				default:
				cout << "\n ArgSpecT::WriteValue: unknown type: " << fType << endl;
				throw eGeneralFail;	
		}
	}
}
