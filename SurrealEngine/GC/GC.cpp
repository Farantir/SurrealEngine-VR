
#include "Precomp.h"
#include "GC.h"

static GCRootNode* roots;
static GCAllocation* allocations;
static GCStats stats;
static std::vector<GC::RootMarker> rootMarkers;

GCRootNode::GCRootNode()
{
	// roots is the head of the list and Collect walks it via next
	next = roots;
	if (next)
		next->prev = this;
	roots = this;
}

GCRootNode::~GCRootNode()
{
	if (prev)
	{
		prev->next = next;
	}
	else
	{
		roots = next;
	}

	if (next)
	{
		next->prev = prev;
	}
}

GCAllocation* GC::GetAllocations()
{
	return allocations;
}

GCAllocation* GC::AllocMemory(size_t size)
{
	size_t memsize = sizeof(GCAllocation) + size;
	GCAllocation* allocation = (GCAllocation*)calloc(1, memsize);
	if (allocation == nullptr)
		throw std::bad_alloc();
	allocation->allocklistNext = allocations;
	allocation->memsize = memsize;
	allocation->unreferencedFlag = true;
	allocations = allocation;
	stats.numObjects++;
	stats.memoryUsage += memsize;
	return allocation;
}

void GC::FreeMemory(GCAllocation* allocation)
{
	free(allocation);
}

void GC::AddRootMarker(RootMarker marker)
{
	rootMarkers.push_back(marker);
}

GCStats GC::Collect(Mode mode, UnreachableVisitor visitor)
{
	GCAllocation* marklist = nullptr;
	for (GCRootNode* root = roots; root != nullptr; root = root->next)
		marklist = GC::MarkObject(marklist, root->obj);

	for (RootMarker marker : rootMarkers)
		marklist = marker(marklist);

	while (marklist)
	{
		marklist = Mark(marklist);
	}

	return Sweep(mode, visitor);
}

GCStats GC::GetStats()
{
	return stats;
}

void GC::ClearMarks()
{
	for (GCAllocation* cur = allocations; cur != nullptr; cur = cur->allocklistNext)
		cur->unreferencedFlag = true;
}

void GC::AddExternalMemory(size_t size)
{
	stats.memoryUsage += size;
}

void GC::RemoveExternalMemory(size_t size)
{
	stats.memoryUsage -= size;
}

GCAllocation* GC::Mark(GCAllocation* marklist)
{
	GCAllocation* marklistout = nullptr;
	for (GCAllocation* allocation = marklist; allocation != nullptr; allocation = allocation->marklistNext)
	{
		marklistout = allocation->object()->Mark(marklistout);
	}
	return marklistout;
}

GCStats GC::Sweep(Mode mode, UnreachableVisitor visitor)
{
	GCStats swept;
	for (GCAllocation* cur = allocations; cur != nullptr; cur = cur->allocklistNext)
	{
		if (cur->unreferencedFlag)
		{
			swept.numObjects++;
			swept.memoryUsage += cur->memsize + cur->object()->ExternalMemorySize();
			if (visitor)
				visitor(cur->object());
		}
	}

	// Nothing is destroyed, and the marks stay so the caller can inspect what is unreachable.
	if (mode == Mode::MarkOnly)
		return swept;

	// Every unreachable object gets to release what it owns while all of them are still
	// constructed. Destroying them one by one instead would let an object's teardown reach
	// into another object that this same sweep has already destroyed.
	for (GCAllocation* cur = allocations; cur != nullptr; cur = cur->allocklistNext)
	{
		if (cur->unreferencedFlag)
			cur->object()->PreDestruct();
	}

	GCAllocation* prev = nullptr;
	GCAllocation* cur = allocations;
	while (cur)
	{
		if (cur->unreferencedFlag)
		{
			GCAllocation* unreferenced = cur;

			cur = cur->allocklistNext;
			if (prev)
				prev->allocklistNext = cur;
			else
				allocations = cur;

			stats.memoryUsage -= unreferenced->memsize;
			stats.numObjects--;

			unreferenced->object()->~GCObject();
			free(unreferenced);
		}
		else
		{
			cur->unreferencedFlag = true;
			prev = cur;
			cur = cur->allocklistNext;
		}
	}

	return swept;
}
