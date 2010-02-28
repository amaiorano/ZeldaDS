#ifndef ITREE_NODE
#define ITREE_NODE

#include "gslib/Core/Core.h"
#include "gslib/Math/Vector2.h"
#include <vector>
#include <algorithm>

template <typename NodeType>
class ITreeNode
{
public:
	typedef NodeType* NodePtr;
	typedef std::vector<NodePtr> NodePtrList;

	virtual ~ITreeNode() { }

	bool AttachChild(NodePtr pChild)
	{
		if (OnPreAttachChild(pChild))
		{
			m_children.push_back(pChild);
			pChild->m_pParent = GetThisAsChildType();
			
			OnPostAttachChild(pChild);
			return true;
		}
		return false;
	}

	bool DetachChild(NodePtr pChild)
	{
		ASSERT(pChild->m_pParent == GetThisAsChildType()); // Make sure it's my immediate child

		if (OnPreDetachChild(pChild))
		{
			// Simply remove the child from my list of children. If there are no more refs to this child,
			// it will be deleted, in which case it will not reference its own children, which might be
			// deleted, etc.
			typename NodePtrList::iterator iter = std::find(m_children.begin(), m_children.end(), pChild);
			if (iter != m_children.end())
			{
				m_children.erase(iter);
				OnPostDetachChild(pChild);
				return true;
			}
		}

		return false;
	}

	bool DetachFromParent()
	{
		if (m_pParent)
		{
			return m_pParent->DetachChild(GetThisAsChildType());
		}
		return false;
	}

	std::size_t GetNumChildren() const
	{
		return m_children.size();
	}

	NodePtr GetChild(std::size_t idx)
	{
		ASSERT(idx>=0 && idx<m_children.size());
		return m_children[idx];
	}

	const NodePtr GetChild(std::size_t idx) const
	{
		ASSERT(idx>=0 && idx<m_children.size());
		return m_children[idx];
	}

	// May return a NULL SharedPtr
	NodePtr GetParent()
	{
		return m_pParent;
	}

	const NodePtr GetParent() const
	{
		return m_pParent;
	}

protected:
	// Let's child classes do more when a child is added; returns true if child should be
	// added to / removed from the tree (default), or false otherwise.
	virtual bool OnPreAttachChild(NodePtr /*pNode*/) { return true; }
	virtual bool OnPreDetachChild(NodePtr /*pNode*/) { return true; }

	virtual void OnPostAttachChild(NodePtr /*pNode*/) { }
	virtual void OnPostDetachChild(NodePtr /*pNode*/) { }

private:
	NodePtr GetThisAsChildType() { return static_cast<NodePtr>(this); }

	NodePtrList m_children;
	NodePtr m_pParent;
};

#endif // ITREE_NODE
