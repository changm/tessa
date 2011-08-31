/***
 * The value class is a key component in the Tessa IR.
 * Instructions operate on multiple values and can produce multiple values.
 * Values have a type associated with them.
 * Values have no execution semantics.
 */
namespace TessaVM {
	class Value : public MMgc::GCObject {
	private:
		TessaTypeEnum		_type;

	public:
		Value();
		Value(TessaTypeEnum type);
		TessaTypeEnum		getTypeOfResult();
		void				setTypeOfResult(TessaTypeEnum type);
		virtual bool		isInstruction();
	};
}