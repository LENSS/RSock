// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: google/protobuf/unittest_optimize_for.proto

#include <google/protobuf/unittest_optimize_for.pb.h>

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/port.h>
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// This is a temporary google only hack
#ifdef GOOGLE_PROTOBUF_ENFORCE_UNIQUENESS
#include "third_party/protobuf/version.h"
#endif
// @@protoc_insertion_point(includes)
namespace protobuf_unittest {
class TestOptimizedForSizeDefaultTypeInternal {
 public:
  ::google::protobuf::internal::ExplicitlyConstructed<TestOptimizedForSize>
      _instance;
  ::google::protobuf::int32 integer_field_;
  ::google::protobuf::internal::ArenaStringPtr string_field_;
} _TestOptimizedForSize_default_instance_;
class TestRequiredOptimizedForSizeDefaultTypeInternal {
 public:
  ::google::protobuf::internal::ExplicitlyConstructed<TestRequiredOptimizedForSize>
      _instance;
} _TestRequiredOptimizedForSize_default_instance_;
class TestOptionalOptimizedForSizeDefaultTypeInternal {
 public:
  ::google::protobuf::internal::ExplicitlyConstructed<TestOptionalOptimizedForSize>
      _instance;
} _TestOptionalOptimizedForSize_default_instance_;
}  // namespace protobuf_unittest
namespace protobuf_google_2fprotobuf_2funittest_5foptimize_5ffor_2eproto {
void InitDefaultsTestOptimizedForSizeImpl() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

#ifdef GOOGLE_PROTOBUF_ENFORCE_UNIQUENESS
  ::google::protobuf::internal::InitProtobufDefaultsForceUnique();
#else
  ::google::protobuf::internal::InitProtobufDefaults();
#endif  // GOOGLE_PROTOBUF_ENFORCE_UNIQUENESS
  protobuf_google_2fprotobuf_2funittest_2eproto::InitDefaultsForeignMessage();
  {
    void* ptr = &::protobuf_unittest::_TestOptimizedForSize_default_instance_;
    new (ptr) ::protobuf_unittest::TestOptimizedForSize();
    ::google::protobuf::internal::OnShutdownDestroyMessage(ptr);
  }
  ::protobuf_unittest::TestOptimizedForSize::InitAsDefaultInstance();
}

void InitDefaultsTestOptimizedForSize() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &InitDefaultsTestOptimizedForSizeImpl);
}

void InitDefaultsTestRequiredOptimizedForSizeImpl() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

#ifdef GOOGLE_PROTOBUF_ENFORCE_UNIQUENESS
  ::google::protobuf::internal::InitProtobufDefaultsForceUnique();
#else
  ::google::protobuf::internal::InitProtobufDefaults();
#endif  // GOOGLE_PROTOBUF_ENFORCE_UNIQUENESS
  {
    void* ptr = &::protobuf_unittest::_TestRequiredOptimizedForSize_default_instance_;
    new (ptr) ::protobuf_unittest::TestRequiredOptimizedForSize();
    ::google::protobuf::internal::OnShutdownDestroyMessage(ptr);
  }
  ::protobuf_unittest::TestRequiredOptimizedForSize::InitAsDefaultInstance();
}

void InitDefaultsTestRequiredOptimizedForSize() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &InitDefaultsTestRequiredOptimizedForSizeImpl);
}

void InitDefaultsTestOptionalOptimizedForSizeImpl() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

#ifdef GOOGLE_PROTOBUF_ENFORCE_UNIQUENESS
  ::google::protobuf::internal::InitProtobufDefaultsForceUnique();
#else
  ::google::protobuf::internal::InitProtobufDefaults();
#endif  // GOOGLE_PROTOBUF_ENFORCE_UNIQUENESS
  protobuf_google_2fprotobuf_2funittest_5foptimize_5ffor_2eproto::InitDefaultsTestRequiredOptimizedForSize();
  {
    void* ptr = &::protobuf_unittest::_TestOptionalOptimizedForSize_default_instance_;
    new (ptr) ::protobuf_unittest::TestOptionalOptimizedForSize();
    ::google::protobuf::internal::OnShutdownDestroyMessage(ptr);
  }
  ::protobuf_unittest::TestOptionalOptimizedForSize::InitAsDefaultInstance();
}

void InitDefaultsTestOptionalOptimizedForSize() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &InitDefaultsTestOptionalOptimizedForSizeImpl);
}

::google::protobuf::Metadata file_level_metadata[3];

const ::google::protobuf::uint32 TableStruct::offsets[] GOOGLE_PROTOBUF_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::protobuf_unittest::TestOptimizedForSize, _has_bits_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::protobuf_unittest::TestOptimizedForSize, _internal_metadata_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::protobuf_unittest::TestOptimizedForSize, _extensions_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::protobuf_unittest::TestOptimizedForSize, _oneof_case_[0]),
  ~0u,  // no _weak_field_map_
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::protobuf_unittest::TestOptimizedForSize, i_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::protobuf_unittest::TestOptimizedForSize, msg_),
  offsetof(::protobuf_unittest::TestOptimizedForSizeDefaultTypeInternal, integer_field_),
  offsetof(::protobuf_unittest::TestOptimizedForSizeDefaultTypeInternal, string_field_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::protobuf_unittest::TestOptimizedForSize, foo_),
  1,
  0,
  ~0u,
  ~0u,
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::protobuf_unittest::TestRequiredOptimizedForSize, _has_bits_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::protobuf_unittest::TestRequiredOptimizedForSize, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::protobuf_unittest::TestRequiredOptimizedForSize, x_),
  0,
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::protobuf_unittest::TestOptionalOptimizedForSize, _has_bits_),
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::protobuf_unittest::TestOptionalOptimizedForSize, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(::protobuf_unittest::TestOptionalOptimizedForSize, o_),
  0,
};
static const ::google::protobuf::internal::MigrationSchema schemas[] GOOGLE_PROTOBUF_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
  { 0, 10, sizeof(::protobuf_unittest::TestOptimizedForSize)},
  { 14, 20, sizeof(::protobuf_unittest::TestRequiredOptimizedForSize)},
  { 21, 27, sizeof(::protobuf_unittest::TestOptionalOptimizedForSize)},
};

static ::google::protobuf::Message const * const file_default_instances[] = {
  reinterpret_cast<const ::google::protobuf::Message*>(&::protobuf_unittest::_TestOptimizedForSize_default_instance_),
  reinterpret_cast<const ::google::protobuf::Message*>(&::protobuf_unittest::_TestRequiredOptimizedForSize_default_instance_),
  reinterpret_cast<const ::google::protobuf::Message*>(&::protobuf_unittest::_TestOptionalOptimizedForSize_default_instance_),
};

void protobuf_AssignDescriptors() {
  AddDescriptors();
  ::google::protobuf::MessageFactory* factory = NULL;
  AssignDescriptors(
      "google/protobuf/unittest_optimize_for.proto", schemas, file_default_instances, TableStruct::offsets, factory,
      file_level_metadata, NULL, NULL);
}

void protobuf_AssignDescriptorsOnce() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &protobuf_AssignDescriptors);
}

void protobuf_RegisterTypes(const ::std::string&) GOOGLE_PROTOBUF_ATTRIBUTE_COLD;
void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::internal::RegisterAllTypes(file_level_metadata, 3);
}

void AddDescriptorsImpl() {
  InitDefaults();
  static const char descriptor[] GOOGLE_PROTOBUF_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
      "\n+google/protobuf/unittest_optimize_for."
      "proto\022\021protobuf_unittest\032\036google/protobu"
      "f/unittest.proto\"\312\002\n\024TestOptimizedForSiz"
      "e\022\t\n\001i\030\001 \001(\005\022.\n\003msg\030\023 \001(\0132!.protobuf_uni"
      "ttest.ForeignMessage\022\027\n\rinteger_field\030\002 "
      "\001(\005H\000\022\026\n\014string_field\030\003 \001(\tH\000*\t\010\350\007\020\200\200\200\200\002"
      "2@\n\016test_extension\022\'.protobuf_unittest.T"
      "estOptimizedForSize\030\322\t \001(\0052r\n\017test_exten"
      "sion2\022\'.protobuf_unittest.TestOptimizedF"
      "orSize\030\323\t \001(\0132/.protobuf_unittest.TestRe"
      "quiredOptimizedForSizeB\005\n\003foo\")\n\034TestReq"
      "uiredOptimizedForSize\022\t\n\001x\030\001 \002(\005\"Z\n\034Test"
      "OptionalOptimizedForSize\022:\n\001o\030\001 \001(\0132/.pr"
      "otobuf_unittest.TestRequiredOptimizedFor"
      "SizeB\002H\002"
  };
  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
      descriptor, 568);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "google/protobuf/unittest_optimize_for.proto", &protobuf_RegisterTypes);
  ::protobuf_google_2fprotobuf_2funittest_2eproto::AddDescriptors();
}

void AddDescriptors() {
  static GOOGLE_PROTOBUF_DECLARE_ONCE(once);
  ::google::protobuf::GoogleOnceInit(&once, &AddDescriptorsImpl);
}
// Force AddDescriptors() to be called at dynamic initialization time.
struct StaticDescriptorInitializer {
  StaticDescriptorInitializer() {
    AddDescriptors();
  }
} static_descriptor_initializer;
}  // namespace protobuf_google_2fprotobuf_2funittest_5foptimize_5ffor_2eproto
namespace protobuf_unittest {

// ===================================================================

void TestOptimizedForSize::InitAsDefaultInstance() {
  ::protobuf_unittest::_TestOptimizedForSize_default_instance_._instance.get_mutable()->msg_ = const_cast< ::protobuf_unittest::ForeignMessage*>(
      ::protobuf_unittest::ForeignMessage::internal_default_instance());
  ::protobuf_unittest::_TestOptimizedForSize_default_instance_.integer_field_ = 0;
  ::protobuf_unittest::_TestOptimizedForSize_default_instance_.string_field_.UnsafeSetDefault(
      &::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
void TestOptimizedForSize::clear_msg() {
  if (msg_ != NULL) msg_->Clear();
  clear_has_msg();
}
#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int TestOptimizedForSize::kIFieldNumber;
const int TestOptimizedForSize::kMsgFieldNumber;
const int TestOptimizedForSize::kIntegerFieldFieldNumber;
const int TestOptimizedForSize::kStringFieldFieldNumber;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

TestOptimizedForSize::TestOptimizedForSize()
  : ::google::protobuf::Message(), _internal_metadata_(NULL) {
  if (GOOGLE_PREDICT_TRUE(this != internal_default_instance())) {
    ::protobuf_google_2fprotobuf_2funittest_5foptimize_5ffor_2eproto::InitDefaultsTestOptimizedForSize();
  }
  SharedCtor();
  // @@protoc_insertion_point(constructor:protobuf_unittest.TestOptimizedForSize)
}
TestOptimizedForSize::TestOptimizedForSize(const TestOptimizedForSize& from)
  : ::google::protobuf::Message(),
      _internal_metadata_(NULL),
      _has_bits_(from._has_bits_),
      _cached_size_(0) {
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  _extensions_.MergeFrom(from._extensions_);
  if (from.has_msg()) {
    msg_ = new ::protobuf_unittest::ForeignMessage(*from.msg_);
  } else {
    msg_ = NULL;
  }
  i_ = from.i_;
  clear_has_foo();
  switch (from.foo_case()) {
    case kIntegerField: {
      set_integer_field(from.integer_field());
      break;
    }
    case kStringField: {
      set_string_field(from.string_field());
      break;
    }
    case FOO_NOT_SET: {
      break;
    }
  }
  // @@protoc_insertion_point(copy_constructor:protobuf_unittest.TestOptimizedForSize)
}

void TestOptimizedForSize::SharedCtor() {
  _cached_size_ = 0;
  ::memset(&msg_, 0, static_cast<size_t>(
      reinterpret_cast<char*>(&i_) -
      reinterpret_cast<char*>(&msg_)) + sizeof(i_));
  clear_has_foo();
}

TestOptimizedForSize::~TestOptimizedForSize() {
  // @@protoc_insertion_point(destructor:protobuf_unittest.TestOptimizedForSize)
  SharedDtor();
}

void TestOptimizedForSize::SharedDtor() {
  if (this != internal_default_instance()) delete msg_;
  if (has_foo()) {
    clear_foo();
  }
}

void TestOptimizedForSize::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* TestOptimizedForSize::descriptor() {
  ::protobuf_google_2fprotobuf_2funittest_5foptimize_5ffor_2eproto::protobuf_AssignDescriptorsOnce();
  return ::protobuf_google_2fprotobuf_2funittest_5foptimize_5ffor_2eproto::file_level_metadata[kIndexInFileMessages].descriptor;
}

const TestOptimizedForSize& TestOptimizedForSize::default_instance() {
  ::protobuf_google_2fprotobuf_2funittest_5foptimize_5ffor_2eproto::InitDefaultsTestOptimizedForSize();
  return *internal_default_instance();
}

TestOptimizedForSize* TestOptimizedForSize::New(::google::protobuf::Arena* arena) const {
  TestOptimizedForSize* n = new TestOptimizedForSize;
  if (arena != NULL) {
    arena->Own(n);
  }
  return n;
}

void TestOptimizedForSize::clear_foo() {
// @@protoc_insertion_point(one_of_clear_start:protobuf_unittest.TestOptimizedForSize)
  switch (foo_case()) {
    case kIntegerField: {
      // No need to clear
      break;
    }
    case kStringField: {
      foo_.string_field_.DestroyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
      break;
    }
    case FOO_NOT_SET: {
      break;
    }
  }
  _oneof_case_[0] = FOO_NOT_SET;
}


void TestOptimizedForSize::Swap(TestOptimizedForSize* other) {
  if (other == this) return;
  InternalSwap(other);
}
void TestOptimizedForSize::InternalSwap(TestOptimizedForSize* other) {
  using std::swap;
  GetReflection()->Swap(this, other);}

::google::protobuf::Metadata TestOptimizedForSize::GetMetadata() const {
  protobuf_google_2fprotobuf_2funittest_5foptimize_5ffor_2eproto::protobuf_AssignDescriptorsOnce();
  return ::protobuf_google_2fprotobuf_2funittest_5foptimize_5ffor_2eproto::file_level_metadata[kIndexInFileMessages];
}


// ===================================================================

void TestRequiredOptimizedForSize::InitAsDefaultInstance() {
}
#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int TestRequiredOptimizedForSize::kXFieldNumber;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

TestRequiredOptimizedForSize::TestRequiredOptimizedForSize()
  : ::google::protobuf::Message(), _internal_metadata_(NULL) {
  if (GOOGLE_PREDICT_TRUE(this != internal_default_instance())) {
    ::protobuf_google_2fprotobuf_2funittest_5foptimize_5ffor_2eproto::InitDefaultsTestRequiredOptimizedForSize();
  }
  SharedCtor();
  // @@protoc_insertion_point(constructor:protobuf_unittest.TestRequiredOptimizedForSize)
}
TestRequiredOptimizedForSize::TestRequiredOptimizedForSize(const TestRequiredOptimizedForSize& from)
  : ::google::protobuf::Message(),
      _internal_metadata_(NULL),
      _has_bits_(from._has_bits_),
      _cached_size_(0) {
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  x_ = from.x_;
  // @@protoc_insertion_point(copy_constructor:protobuf_unittest.TestRequiredOptimizedForSize)
}

void TestRequiredOptimizedForSize::SharedCtor() {
  _cached_size_ = 0;
  x_ = 0;
}

TestRequiredOptimizedForSize::~TestRequiredOptimizedForSize() {
  // @@protoc_insertion_point(destructor:protobuf_unittest.TestRequiredOptimizedForSize)
  SharedDtor();
}

void TestRequiredOptimizedForSize::SharedDtor() {
}

void TestRequiredOptimizedForSize::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* TestRequiredOptimizedForSize::descriptor() {
  ::protobuf_google_2fprotobuf_2funittest_5foptimize_5ffor_2eproto::protobuf_AssignDescriptorsOnce();
  return ::protobuf_google_2fprotobuf_2funittest_5foptimize_5ffor_2eproto::file_level_metadata[kIndexInFileMessages].descriptor;
}

const TestRequiredOptimizedForSize& TestRequiredOptimizedForSize::default_instance() {
  ::protobuf_google_2fprotobuf_2funittest_5foptimize_5ffor_2eproto::InitDefaultsTestRequiredOptimizedForSize();
  return *internal_default_instance();
}

TestRequiredOptimizedForSize* TestRequiredOptimizedForSize::New(::google::protobuf::Arena* arena) const {
  TestRequiredOptimizedForSize* n = new TestRequiredOptimizedForSize;
  if (arena != NULL) {
    arena->Own(n);
  }
  return n;
}

void TestRequiredOptimizedForSize::Swap(TestRequiredOptimizedForSize* other) {
  if (other == this) return;
  InternalSwap(other);
}
void TestRequiredOptimizedForSize::InternalSwap(TestRequiredOptimizedForSize* other) {
  using std::swap;
  GetReflection()->Swap(this, other);}

::google::protobuf::Metadata TestRequiredOptimizedForSize::GetMetadata() const {
  protobuf_google_2fprotobuf_2funittest_5foptimize_5ffor_2eproto::protobuf_AssignDescriptorsOnce();
  return ::protobuf_google_2fprotobuf_2funittest_5foptimize_5ffor_2eproto::file_level_metadata[kIndexInFileMessages];
}


// ===================================================================

void TestOptionalOptimizedForSize::InitAsDefaultInstance() {
  ::protobuf_unittest::_TestOptionalOptimizedForSize_default_instance_._instance.get_mutable()->o_ = const_cast< ::protobuf_unittest::TestRequiredOptimizedForSize*>(
      ::protobuf_unittest::TestRequiredOptimizedForSize::internal_default_instance());
}
#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int TestOptionalOptimizedForSize::kOFieldNumber;
#endif  // !defined(_MSC_VER) || _MSC_VER >= 1900

TestOptionalOptimizedForSize::TestOptionalOptimizedForSize()
  : ::google::protobuf::Message(), _internal_metadata_(NULL) {
  if (GOOGLE_PREDICT_TRUE(this != internal_default_instance())) {
    ::protobuf_google_2fprotobuf_2funittest_5foptimize_5ffor_2eproto::InitDefaultsTestOptionalOptimizedForSize();
  }
  SharedCtor();
  // @@protoc_insertion_point(constructor:protobuf_unittest.TestOptionalOptimizedForSize)
}
TestOptionalOptimizedForSize::TestOptionalOptimizedForSize(const TestOptionalOptimizedForSize& from)
  : ::google::protobuf::Message(),
      _internal_metadata_(NULL),
      _has_bits_(from._has_bits_),
      _cached_size_(0) {
  _internal_metadata_.MergeFrom(from._internal_metadata_);
  if (from.has_o()) {
    o_ = new ::protobuf_unittest::TestRequiredOptimizedForSize(*from.o_);
  } else {
    o_ = NULL;
  }
  // @@protoc_insertion_point(copy_constructor:protobuf_unittest.TestOptionalOptimizedForSize)
}

void TestOptionalOptimizedForSize::SharedCtor() {
  _cached_size_ = 0;
  o_ = NULL;
}

TestOptionalOptimizedForSize::~TestOptionalOptimizedForSize() {
  // @@protoc_insertion_point(destructor:protobuf_unittest.TestOptionalOptimizedForSize)
  SharedDtor();
}

void TestOptionalOptimizedForSize::SharedDtor() {
  if (this != internal_default_instance()) delete o_;
}

void TestOptionalOptimizedForSize::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* TestOptionalOptimizedForSize::descriptor() {
  ::protobuf_google_2fprotobuf_2funittest_5foptimize_5ffor_2eproto::protobuf_AssignDescriptorsOnce();
  return ::protobuf_google_2fprotobuf_2funittest_5foptimize_5ffor_2eproto::file_level_metadata[kIndexInFileMessages].descriptor;
}

const TestOptionalOptimizedForSize& TestOptionalOptimizedForSize::default_instance() {
  ::protobuf_google_2fprotobuf_2funittest_5foptimize_5ffor_2eproto::InitDefaultsTestOptionalOptimizedForSize();
  return *internal_default_instance();
}

TestOptionalOptimizedForSize* TestOptionalOptimizedForSize::New(::google::protobuf::Arena* arena) const {
  TestOptionalOptimizedForSize* n = new TestOptionalOptimizedForSize;
  if (arena != NULL) {
    arena->Own(n);
  }
  return n;
}

void TestOptionalOptimizedForSize::Swap(TestOptionalOptimizedForSize* other) {
  if (other == this) return;
  InternalSwap(other);
}
void TestOptionalOptimizedForSize::InternalSwap(TestOptionalOptimizedForSize* other) {
  using std::swap;
  GetReflection()->Swap(this, other);}

::google::protobuf::Metadata TestOptionalOptimizedForSize::GetMetadata() const {
  protobuf_google_2fprotobuf_2funittest_5foptimize_5ffor_2eproto::protobuf_AssignDescriptorsOnce();
  return ::protobuf_google_2fprotobuf_2funittest_5foptimize_5ffor_2eproto::file_level_metadata[kIndexInFileMessages];
}

#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int TestOptimizedForSize::kTestExtensionFieldNumber;
#endif
::google::protobuf::internal::ExtensionIdentifier< ::protobuf_unittest::TestOptimizedForSize,
    ::google::protobuf::internal::PrimitiveTypeTraits< ::google::protobuf::int32 >, 5, false >
  TestOptimizedForSize::test_extension(kTestExtensionFieldNumber, 0);
#if !defined(_MSC_VER) || _MSC_VER >= 1900
const int TestOptimizedForSize::kTestExtension2FieldNumber;
#endif
::google::protobuf::internal::ExtensionIdentifier< ::protobuf_unittest::TestOptimizedForSize,
    ::google::protobuf::internal::MessageTypeTraits< ::protobuf_unittest::TestRequiredOptimizedForSize >, 11, false >
  TestOptimizedForSize::test_extension2(kTestExtension2FieldNumber, *::protobuf_unittest::TestRequiredOptimizedForSize::internal_default_instance());

// @@protoc_insertion_point(namespace_scope)
}  // namespace protobuf_unittest

// @@protoc_insertion_point(global_scope)
