#pragma once

#include "Foundation/SmartPtr.h"
#include "Foundation/Event.h"
#include "Foundation/Property.h"
#include "Application/UndoQueue.h"

#include "Reflect/TranslatorDeduction.h"

#include "Inspect/API.h"

#include <iomanip>

#define INSPECT_BASE(__ID, __Type) \
public: \
typedef __Type This; \
virtual int GetType() const \
{ \
	return __ID; \
} \
virtual bool HasType(int id) const \
{ \
	return __ID == id; \
}

#define INSPECT_TYPE(__ID, __Type, __Base) \
public: \
typedef __Type This; \
typedef __Base Base; \
virtual int GetType() const \
{ \
	return __ID; \
} \
virtual bool HasType(int id) const \
{ \
	return __ID == id || Base::HasType(id); \
}

namespace Helium
{
	namespace Inspect
	{
		//
		// Constants
		//

		const char UNDEF_VALUE_STRING[] = "Undef";
		const char MULTI_VALUE_STRING[] = "Multi";

		//
		// Extract and Insert are template functions that go between a sting buffer and typed data
		//

		template< class T > void Extract(std::istream& stream, T* val);
		template< class T > void Insert(std::ostream& stream, const T* val);

		//
		// Specialize std::string data as this requires special support for empty strings
		//

		template<>
		HELIUM_INSPECT_API void Extract(std::istream& stream, std::string* val);

		//
		// Treat chars as numbers
		//

		template<>
		HELIUM_INSPECT_API void Extract(std::istream& stream, uint8_t* val);
		template<>
		HELIUM_INSPECT_API void Insert(std::ostream& stream, const uint8_t* val);

		template<>
		HELIUM_INSPECT_API void Extract(std::istream& stream, int8_t* val);
		template<>
		HELIUM_INSPECT_API void Insert(std::ostream& stream, const int8_t* val);

		//
		// Used fixed notation for floating point
		//

		template<>
		HELIUM_INSPECT_API void Insert(std::ostream& stream, const float32_t* val);
		template<>
		HELIUM_INSPECT_API void Insert(std::ostream& stream, const float64_t* val);

		//
		// Reflect Data support
		//

		template<>
		HELIUM_INSPECT_API void Extract(std::istream& stream, Reflect::Data* val);
		template<>
		HELIUM_INSPECT_API void Insert(std::ostream& stream, const Reflect::Data* val);

		//
		// DataBinding base class
		//

		class DataBinding;

		struct DataChangingArgs
		{
			inline DataChangingArgs( const DataBinding* data, Reflect::Data value );

			const DataBinding* m_Data;
			Reflect::Data      m_NewValue;
			mutable bool       m_Veto;
		};
		typedef Helium::Signature< const DataChangingArgs& > DataChangingSignature;

		struct DataChangedArgs
		{
			inline DataChangedArgs( const DataBinding* data );

			const DataBinding* m_Data;
		};
		typedef Helium::Signature< const DataChangedArgs& > DataChangedSignature;

		namespace DataBindingTypes
		{
			enum DataBindingType
			{
				Custom,
				String,
				Typed,
			};
		}

		//
		// Base class for all types of data binding with undo, event signaling, and virtual desruction
		//

		class DataBinding : public Helium::RefCountBase< DataBinding >
		{
		public:
			INSPECT_BASE( DataBindingTypes::Custom, DataBinding );

			inline DataBinding();
			inline virtual ~DataBinding();
			virtual void Refresh() = 0;
			virtual UndoCommandPtr GetUndoCommand() const = 0;

		protected: 
			bool m_Significant; 
		public: 
			inline void SetSignificant(bool significant);
			inline bool IsSignificant() const;

		protected:
			mutable DataChangingSignature::Event m_Changing;
		public:
			inline void AddChangingListener( const DataChangingSignature::Delegate& listener ) const;
			inline void RemoveChangingListener( const DataChangingSignature::Delegate& listener ) const;

		protected:
			mutable DataChangedSignature::Event m_Changed;
		public:
			inline void AddChangedListener( const DataChangedSignature::Delegate& listener ) const;
			inline void RemoveChangedListener( const DataChangedSignature::Delegate& listener ) const;
		};

		typedef Helium::SmartPtr<DataBinding> DataBindingPtr;

		//
		// Casting function for different classes for data bindings
		//

		template< typename T, DataBindingTypes::DataBindingType type >
		inline T* CastDataBinding( DataBinding* data );

		//
		// Base template for data, V is the value container, which may or may not be equal to T
		//  it will not be equal if insertion or extraction translates a compiler type to a string
		//

		template<class T>
		class DataBindingCommand;

		template<class T>
		class DataBindingTemplate : public DataBinding
		{
		public:
			typedef Helium::SmartPtr< DataBindingTemplate > Ptr;

		public:
			virtual void Refresh() override;
			virtual UndoCommandPtr GetUndoCommand() const override;

			virtual bool Set(const T& s, const DataChangedSignature::Delegate& emitter = DataChangedSignature::Delegate ()) = 0;
			virtual bool SetAll(const std::vector<T>& s, const DataChangedSignature::Delegate& emitter = DataChangedSignature::Delegate ());

			virtual void Get(T& s) const = 0;
			virtual void GetAll(std::vector<T>& s) const;
		};

		//
		// UndoCommand object for DataBinding Undo/Redo
		//  Store state of object(s) bound by data
		//

		template<class T>
		class DataBindingCommand : public UndoCommand
		{
		protected:
			// the data object that is used to read/write from the client objects
			typename DataBindingTemplate<T>::Ptr m_Data;

			// state information
			std::vector<T> m_Values;

		public:
			DataBindingCommand( const typename DataBindingTemplate<T>::Ptr& data );

			virtual void Undo() override;
			virtual void Redo() override;
			virtual bool IsSignificant() const override;

		private:
			void Swap();
		};


		//
		// Base class for all string-translated data types
		//

		class StringDataBinding : public DataBindingTemplate< std::string >
		{
		public:
			INSPECT_TYPE( DataBindingTypes::String, StringDataBinding, DataBinding );
		};
		typedef Helium::SmartPtr< StringDataBinding > StringDataPtr;

		//
		// StringFormatter handles conversion between T and string
		//

		template<class T>
		class StringFormatter : public StringDataBinding
		{
		public:
			typedef Helium::SmartPtr< StringFormatter<T> > Ptr;

		protected:
			// the data we are manipulating
			T* m_Data;

			// perishable data gets thrown away at destruction
			bool m_Perishable;

		public:
			StringFormatter(T* data, bool perishable = false);
			virtual ~StringFormatter();

			virtual bool Set(const std::string& s, const DataChangedSignature::Delegate& emitter = NULL) override;
			virtual void Get(std::string& s) const override;
		};

		//
		// MultiStringFormatter handles conversion between multiple T's and strings
		//

		template<class T>
		class MultiStringFormatter : public StringDataBinding
		{
		public:
			typedef Helium::SmartPtr< MultiStringFormatter<T> > Ptr;

		protected:
			// a link to each data we are manipulating
			std::vector<T*> m_Data;

			// perishable data gets thrown away at destruction
			bool m_Perishable;

		public:
			MultiStringFormatter( const std::vector<T*>& data, bool perishable = false);
			virtual ~MultiStringFormatter();

			virtual bool Set(const std::string& s, const DataChangedSignature::Delegate& emitter = NULL) override;
			virtual bool SetAll(const std::vector< std::string >& values, const DataChangedSignature::Delegate& emitter = NULL) override;

			virtual void Get(std::string& s) const override;
			virtual void GetAll(std::vector< std::string >& s) const override;
		};

		//
		// PropertyStringFormatter handles conversion between a property of T and string
		//

		template<class T>
		class PropertyStringFormatter : public StringDataBinding
		{
		public:
			typedef Helium::SmartPtr< PropertyStringFormatter<T> > Ptr;

		protected:
			Helium::SmartPtr< Helium::Property<T> > m_Property;

		public:
			PropertyStringFormatter(const Helium::SmartPtr< Helium::Property<T> >& property);
			virtual ~PropertyStringFormatter();

			virtual bool Set(const std::string& s, const DataChangedSignature::Delegate& emitter = NULL) override;
			virtual void Get(std::string& s) const override;
		};

		//
		// MultiPropertyStringFormatter handles conversion between T and string
		//

		template<class T>
		class MultiPropertyStringFormatter : public StringDataBinding
		{
		public:
			typedef Helium::SmartPtr< MultiPropertyStringFormatter<T> > Ptr;

		protected:
			std::vector< Helium::SmartPtr< Helium::Property<T> > > m_Properties;

		public:
			MultiPropertyStringFormatter(const std::vector< Helium::SmartPtr< Helium::Property<T> > >& properties);
			virtual ~MultiPropertyStringFormatter();

			virtual bool Set(const std::string& s, const DataChangedSignature::Delegate& emitter = NULL) override;
			virtual bool SetAll(const std::vector< std::string >& s, const DataChangedSignature::Delegate& emitter = NULL) override;

			virtual void Get(std::string& s) const override;
			virtual void GetAll(std::vector< std::string >& s) const override;
		};


		// 
		// Base class for all data data types
		// 

		template< class T >
		class TypedDataBinding : public DataBindingTemplate< T >
		{
		public:
			INSPECT_TYPE( DataBindingTypes::Typed, TypedDataBinding, DataBinding );
		};

		//
		// TypedPropertyFormatter handles conversion between a property of T and string
		//

		template< class T >
		class TypedPropertyFormatter : public TypedDataBinding< T >
		{
		protected:
			Helium::SmartPtr< Helium::Property< T > > m_Property;

		public:
			TypedPropertyFormatter(const Helium::SmartPtr< Helium::Property< T > >& property);
			virtual ~TypedPropertyFormatter();

			virtual bool Set(const T& value, const DataChangedSignature::Delegate& emitter = NULL) override;
			virtual void Get(T& value) const override;
		};
	}
}

#include "Inspect/DataBinding.inl"
