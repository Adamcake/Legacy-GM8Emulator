#pragma once

struct GlobalValues;
struct GMLType;
enum struct GMLTypeState;
typedef unsigned int CodeObject;
typedef unsigned int InstanceID;
typedef unsigned int InstanceHandle;

namespace CodeManager {
    bool Init(GlobalValues* globals);
    void Finalize();

    // For populating the constant lists. This should be called before anything else.
    bool Init();

    // Set a specific room order. Usually done after loading the room order from the exe.
    void SetRoomOrder(unsigned int** order, unsigned int count);

    // Register a code block to be compiled. Returns a unique reference to that code action to be later Compile()d and Run().
    CodeObject Register(const char* code, unsigned int length);

    // Register a code expression to be compiled. Returns a unique reference to that code action to be later Compile()d and Query()d.
    CodeObject RegisterQuestion(const char* code, unsigned int length);

    // Compile a code object that has been returned by Register(). Returns true on success, false on error (ie. game should close.)
    // Be sure to call this only after the AssetManager is fully loaded.
    bool Compile(CodeObject object);

    // Run a compiled code object. Returns true on success, false on error (ie. the game should close.)
    // Most be passed the instance ID of the "self" and "other" instances in this context. (both may be NULL)
    // ev and sub indicate the event that's being run. For more info, check the "COMPILED OBJECT EVENTS" section of notes.txt
    bool Run(CodeObject code, InstanceHandle self, InstanceHandle other, int ev, int sub, unsigned int asObjId, unsigned int argc = 0, GMLType* argv = nullptr);

    // Run a compiled GML question (boolean expression). Returns true on success, false on error (ie. the game should close.)
    // The output value is stored in the supplied pointer.
    bool Query(CodeObject code, InstanceHandle self, InstanceHandle other, int ev, int sub, unsigned int asObjId, bool* response, unsigned int argc = 0, GMLType* argv = nullptr);
    bool Query(CodeObject code, InstanceHandle self, InstanceHandle other, int ev, int sub, unsigned int asObjId, GMLType* response);

    // Checks if there was a runtime error and, if so, gets the associated error message
    bool GetError(const char** err);
};
