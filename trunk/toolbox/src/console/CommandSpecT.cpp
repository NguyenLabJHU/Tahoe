/* $Id: CommandSpecT.cpp,v 1.4 2001-12-13 02:58:38 paklein Exp $ */

#include "CommandSpecT.h"
#include "ArgSpecT.h"

/* array copy behavior */
const bool ArrayT<CommandSpecT*>::fByteCopy = true; 
const bool ArrayT<CommandSpecT>::fByteCopy = false; 

CommandSpecT::CommandSpecT(const StringT& name, bool ordered_args):
	fName(name),
	fOrdered(ordered_args),
	fArguments(0),
	fPrompter(NULL)
{

}

/* copy constructor */
CommandSpecT::CommandSpecT(const CommandSpecT& command):
	fName(command.Name()),
	fOrdered(command.Ordered()),
	fArguments(0),
	fPrompter(command.Prompter())
{
	/* copy argument list */
	const ArrayT<ArgSpecT*>& args = command.Arguments();
	for (int i = 0; i < args.Length(); i++)
		AddArgument(*(args[i]));
}

/* destructor */
CommandSpecT::~CommandSpecT(void)
{
	for (int i = 0; i < fArguments.Length(); i++)
		delete fArguments[i];
}

/* return a reference to a specific argument */
ArgSpecT& CommandSpecT::Argument(const char* name)
{
	int index = -1;
	for (int i = 0; index == -1 && i < fArguments.Length(); i++)
		if (fArguments[i]->Name() == name)
			index = i;
	if (index == -1) {
		cout << "CommandSpecT::Argument: not found \"" << name << '\"' << endl;
		throw eGeneralFail;
	}
	return Argument(index);
}

/* return a reference to a specific argument */
const ArgSpecT& CommandSpecT::Argument(const char* name) const
{
	int index = -1;
	for (int i = 0; index == -1 && i < fArguments.Length(); i++)
		if (fArguments[i]->Name() == name)
			index = i;
	if (index == -1) {
		cout << "CommandSpecT::Argument: not found \"" << name << '\"' << endl;
		throw eGeneralFail;
	}
	return Argument(index);
}

/* add an argument to the function */
void CommandSpecT::AddArgument(const ArgSpecT& arg)
{
	/* check - unnamed must be ordered */
	if (!fOrdered && arg.Name().StringLength() == 0)
	{
		cout << "\n CommandSpecT::AddArgument: unordered arguments must be named" << endl;
		throw eGeneralFail;
	}

	/* new argument spec */
	ArgSpecT* new_arg = new ArgSpecT(arg);

	/* store */
	fArguments.Append(new_arg);
}

/* clear all argument values */
void CommandSpecT::ClearValues(void)
{
	for (int i = 0; i < fArguments.Length(); i++)
	fArguments[i]->ClearValue();
}

/* write function spec to output stream */
void CommandSpecT::Write(ostream& out) const
{
	/* function name */
	out << fName << ": " << fArguments.Length();
	if (fArguments.Length() == 1)
		out << " argument";
	else
		out << " arguments";
		
	if (fArguments.Length() > 1)
	{
		if (fOrdered)
			out << ": ordered\n";
		else
			out << ": not ordered\n";
	}
	else out << '\n';	
		
	/* arguments */
	for (int i = 0; i < fArguments.Length(); i++)
	{
		fArguments[i]->Write(out);
		out << '\n';
	}

	out.flush();
}

/* write command statement to the output stream */
void CommandSpecT::WriteCommand(ostream& out) const
{
	/* function name */
	out << fName << " ";
	for (int i = 0; i < fArguments.Length(); i++)
	{
		/* argument name */
		if (fArguments[i]->Name().StringLength() > 0)
			out << fArguments[i]->Name() << " ";
			
		/* value */
		fArguments[i]->WriteValue(out);
		out << " ";
	}
}
