#ifndef Node_h__
#define Node_h__

struct Node
{
	double G;
	double H;
	Node* Parent;
	float X;
	float Y;

	Node(Node* parent, float x, float y, double g, double h) : Parent(parent), X(x), Y(y), G(g), H(h) {}

	int CompareTo(Node* other)
	{
		return (int)(G + H - (other->G + other->H));
	}

	bool Equals(Node* other)
	{
		if (other == nullptr) return false;

		if ((G + H - (other->G + other->H)) == 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
};

#endif // !Node_h__