#ifndef Node_h__
#define Node_h__

struct Node
{
	double G;
	double H;
	Node* Parent;
	int X;
	int Y;
	bool Solid;

	Node(int x, int y, bool solid) : Parent(nullptr), X(x), Y(y), Solid(solid), G(0.0), H(0.0) {}

	Node(Node* parent, int x, int y, bool solid, double g, double h) : Parent(parent), X(x), Y(y), Solid(solid), G(g), H(h) {}

	static bool CompareNodeToNode(Node* a, Node* b)
	{
		return (a->G + a->H) < (b->G + b->H);
	}
};

#endif // !Node_h__