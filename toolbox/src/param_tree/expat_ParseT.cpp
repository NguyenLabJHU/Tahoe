/* $Id: expat_ParseT.cpp,v 1.9 2004-07-08 22:55:43 paklein Exp $ */
#include "expat_ParseT.h"
#ifdef __EXPAT__

#include "ParameterListT.h"

/* C file I/O header */
#if defined(__SGI__)
#include <stdio.h>
#else
//what do others use???
#endif

using namespace Tahoe;

/* init static values */
AutoArrayT<ParameterListT*> expat_ParseT::sListStack;

/* constructor */
expat_ParseT::expat_ParseT(void)
{
	/* stack should be empty */
	if (sListStack.Length() != 0)
		ExceptionT::GeneralFail("expat_ParseT::expat_ParseT", "stack depth should be 0: %d", sListStack.Length());
}

/* parse the given file placing values into the given parameter tree */
void expat_ParseT::Parse(const StringT& file, ParameterListT& params)
{
	const char caller[] = "expat_ParseT::Parse";

	/* check stack */
	if (sListStack.Length() != 0)
		ExceptionT::GeneralFail(caller, "list stack is not empty");
	sListStack.Append(&params);

	/* create parser */
	XML_Parser parser = XML_ParserCreate(NULL);

	/* disable parsing of external entities, i.e., the DTD */
	XML_SetParamEntityParsing(parser, XML_PARAM_ENTITY_PARSING_NEVER);

	/* set element handlers */
	XML_SetElementHandler(parser, startElement, endElement);

	ExceptionT::CodeT error = ExceptionT::kNoError;
	FILE* fp = NULL;
	try {
	
	/* open a file */
	fp = fopen(file, "r");
	if (!fp) ExceptionT::GeneralFail(caller, "error opening file \"%s\"", file.Pointer());

	/* parse */
	int done;
	char buf[BUFSIZ];
  	do {
		size_t len = fread(buf, 1, sizeof(buf), fp);
		done = len < sizeof(buf);

		/* handle error */
		if (XML_Parse(parser, buf, len, done) == XML_STATUS_ERROR) {

			/* get buffer */
			int offset = 0;
			int size = 0;
			const char* buffer = XML_GetInputContext(parser, &offset, &size);
			buffer += offset;

			/* report error message */
			printf(
				"\n expat error: \"%s\" at line %d near\n%s",
				XML_ErrorString(XML_GetErrorCode(parser)),
				XML_GetCurrentLineNumber(parser), buffer);

			ExceptionT::BadInputValue(caller, "error parsing file \"%s\"", file.Pointer());
    	}

	} while (!done);

	/* stack check */
	if (sListStack.Length() != 1)
		ExceptionT::GeneralFail(caller, "stack depth should be 1: %d", sListStack.Length());

	} /* end try */
	
	catch(ExceptionT::CodeT err) {
		error = err;
	}

	/* clean up */
	if (fp) fclose(fp);
	XML_ParserFree(parser);
	sListStack.Dimension(0);
	
	/* error */
	if (error != ExceptionT::kNoError)
		ExceptionT::Throw(error, caller, "failed");
}

/**********************************************************************
 * Private
 **********************************************************************/

void expat_ParseT::startElement(void *userData, const char *name, const char **atts)
{
#pragma unused(userData)
	const char caller[] = "expat_ParseT::startElement";

	/* check length */
	if (sListStack.Length() < 1)
		ExceptionT::GeneralFail(caller, "stack is empty");

	/* parent list */
	ParameterListT* parent = sListStack.Last();

	/* new sublist */
	ParameterListT sublist(name);
	sublist.SetDuplicateListNames(parent->DuplicateListNames());
	
	/* put attributes into the list */
	while (*atts != NULL) {
	
		const char* att_name = *atts++;
		if (*atts == NULL) 
			ExceptionT::GeneralFail(caller, "unexpected end of attributes after \"%s\"", att_name);
		const char* att_value = *atts++;

		if (strcmp(att_name, "description") == 0)
			sublist.SetDescription(att_value);
		else
		{
			/* add to the list */
			ParameterT param(att_value, att_name);
			if (!sublist.AddParameter(param))
				ExceptionT::BadInputValue(caller, "could not add parameter \"%s\" to list \"%s\"",
					param.Name().Pointer(), sublist.Name().Pointer());
		}
	}

	/* append sublist to parent */
	if (!parent->AddList(sublist))
		ExceptionT::BadInputValue(caller, "could not add sublist \"%s\" to list \"%s\"",
			sublist.Name().Pointer(), parent->Name().Pointer());

	/* put newest sublist at the end of the stack */
	const ArrayT<ParameterListT>& subs = parent->Lists();
	const ParameterListT& last = subs.Last();
	if (last.Name() != name)
		ExceptionT::GeneralFail(caller, "last list should be \"%s\" not \"%s\"", 
			name, last.Name().Pointer());
	sListStack.Append((ParameterListT*) &last);
}

void expat_ParseT::endElement(void *userData, const char *name)
{
#pragma unused(userData)
	const char caller[] = "expat_ParseT::startElement";

	/* check length */
	if (sListStack.Length() < 2)
		ExceptionT::GeneralFail(caller, "stack too short %d", sListStack.Length());

	/* pull from stack */
	ParameterListT* list = sListStack.Last();
	if (list->Name() != name)
		ExceptionT::GeneralFail(caller, "expecting \"%s\" not \"%s\" in stack",
			name, list->Name().Pointer());
	sListStack.DeleteAt(sListStack.Length() - 1);
}

#endif /* __EXPAT__ */
