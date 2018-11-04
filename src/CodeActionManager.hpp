#ifndef _A_CODEACTION_HPP_
#define _A_CODEACTION_HPP_
#include <vector>

typedef unsigned int CodeAction;
typedef unsigned int CodeObject;
class CodeRunner;
struct Instance;
class Object;

namespace CodeActionManager {
    bool Init();
    void Finalize();

    // Read code action from EXE data stream, prepares and registers a GML block with the code runner.
    // Returns true on success, false on error (ie. game should close.) Outputs CodeAction reference in the "out" param.
    bool Read(const unsigned char* stream, unsigned int* pos, CodeAction* out);

    // Compile an action into GML and store code reference. Returns true on success, false on error (ie. game should close.)
    // Only do this after the asset list is fully loaded.
    bool Compile(CodeAction action);

    // Run a list of actions. Returns true on success, false on error (ie. game should close.)
    bool Run(CodeAction* actions, unsigned int count, Instance* self, Instance* other, int ev, int sub, unsigned int asObjId);

    // Run an event that belongs to a certain instance. This emulates GML's event_perform().
    // This needs to be the responsibility of the CodeActionManager because sometimes CodeRuner and CodeActionManager
    // need to perform their own events, such as from within event_perform() and event_inherited().
    // Also see "COMPILED OBJECT EVENTS" in notes.txt for more information on ev and sub parameters.
    bool RunInstanceEvent(int ev, int sub, Instance* target, Instance* other, unsigned int asObjId);

    // In conjunction with the above, this tells us if an ev and sub has a defined event within an object.
    // If not, the instance's parent should usually be checked next.
    bool CheckObjectEvent(int ev, int sub, Object* obj);
};

#endif