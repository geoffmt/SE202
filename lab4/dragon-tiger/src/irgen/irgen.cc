#include "irgen.hh"
#include "../utils/errors.hh"

#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"

using utils::error;

namespace irgen {

IRGenerator::IRGenerator() : Builder(Context) {
  Mod = llvm::make_unique<llvm::Module>("tiger", Context);
}

llvm::Type *IRGenerator::llvm_type(const ast::Type ast_type) {
  switch (ast_type) {
  case t_int:
    return Builder.getInt32Ty();
  case t_string:
    return Builder.getInt8PtrTy();
  case t_void:
    return Builder.getVoidTy();
  default:
    assert(false); __builtin_unreachable();
  }
}

llvm::Value *IRGenerator::alloca_in_entry(llvm::Type *Ty,
                                          const std::string &name) {
  llvm::IRBuilderBase::InsertPoint const saved = Builder.saveIP();
  Builder.SetInsertPoint(&current_function->getEntryBlock());
  llvm::Value *const value = Builder.CreateAlloca(Ty, nullptr, name);
  Builder.restoreIP(saved);
  return value;
}

void IRGenerator::print_ir(std::ostream *ostream) {
  // FIXME: This is inefficient. Should probably take a filename
  // and use directly LLVM raw stream interface
  std::string buffer;
  llvm::raw_string_ostream OS(buffer);
  OS << *Mod;
  OS.flush();
  *ostream << buffer;
}

llvm::Value *IRGenerator::address_of(const Identifier &id)
{
  assert(id.get_decl());
  const VarDecl &decl = dynamic_cast<const VarDecl &>(id.get_decl().get());
  // variable used at the same depth
  if (!decl.get_escapes())
  {
    return allocations[&decl];
  }
  else
  {
    // use of frame_up to find the address of the function
    std::pair<llvm::StructType *, llvm::Value *> fu = frame_up(id.get_depth() - decl.get_depth());
    llvm::StructType * ft = fu.first;
    llvm::Value * sl = fu.second;
    int pos = frame_position[&decl];

    return Builder.CreateStructGEP(ft, sl, pos);
  }
}

void IRGenerator::generate_program(FunDecl *main) {
  main->accept(*this);

  while (!pending_func_bodies.empty()) {
    generate_function(*pending_func_bodies.back());
    pending_func_bodies.pop_back();
  }
}

void IRGenerator::generate_function(const FunDecl &decl)
{
  // Reinitialize common structures.
  allocations.clear();
  loop_exit_bbs.clear();

  // Set current function
  current_function = Mod->getFunction(decl.get_external_name().get());
  current_function_decl = &decl;
  std::vector<VarDecl *> params = decl.get_params();

  // Create a new basic block to insert allocation insertion
  llvm::BasicBlock *bb1 =
      llvm::BasicBlock::Create(Context, "entry", current_function);

  Builder.SetInsertPoint(bb1);
  generate_frame();

  // Create a second basic block for body insertion
  llvm::BasicBlock *bb2 =
      llvm::BasicBlock::Create(Context, "body", current_function);

  Builder.SetInsertPoint(bb2);

  // Set the name for each argument and register it in the allocations map
  // after storing it in an alloca.
  unsigned i = 0;
  for (auto &arg : current_function->args())
  {
    // analyzing a non external function
    if (!decl.is_external)
    {
      if (i == 0)
      {
        arg.setName("top");
        Builder.CreateStore(&arg, Builder.CreateStructGEP(frame_type[&decl], frame, 0));
      }
      else
      {
        arg.setName(params[i - 1]->name.get());
        Builder.CreateStore(&arg, generate_vardecl(*params[i - 1]));
      }
    }
    // external function
    else
    {
      arg.setName(params[i]->name.get());
      Builder.CreateStore(&arg, generate_vardecl(*params[i]));
    }
  }

  // Visit the body
  llvm::Value *expr = decl.get_expr()->accept(*this);

  // Finish off the function.
  if (decl.get_type() == t_void)
    Builder.CreateRetVoid();
  else
    Builder.CreateRet(expr);

  // Jump from entry to body
  Builder.SetInsertPoint(bb1);
  Builder.CreateBr(bb2);

  // Validate the generated code, checking for consistency.
  llvm::verifyFunction(*current_function);
}

void IRGenerator::generate_frame()
{
  // Vector of types needed in frame
  std::vector<llvm::Type *> framed_var;

  // first field is a pointer to parent frame
  if (current_function_decl->get_parent())
  {
    framed_var.push_back(frame_type[&current_function_decl->get_parent().value()]->getPointerTo());
  }

  // types of escaping declarations
  for (VarDecl *escaping_decl : current_function_decl->get_escaping_decls())
  {
    if (escaping_decl->get_type() != t_void)
    {
      framed_var.push_back(llvm_type(escaping_decl->get_type()));
    }
  }

  //get external name
  std::string ext_name = std::string(current_function_decl->get_external_name());

  //create ft_ structure
  llvm::StructType *ft_ = llvm::StructType::create(Context, framed_var, "ft_" + ext_name);

  // register
  frame_type[current_function_decl] = ft_;

  // allocate new object on the stack
  frame = Builder.CreateAlloca(ft_, nullptr, "frame_" + ext_name);
}

std::pair<llvm::StructType *, llvm::Value *> IRGenerator::frame_up(int levels)
{

  const FunDecl *fun = current_function_decl;
  llvm::Value *sl = frame;

  for (int i = 0; i < levels; i++)
  {
    sl = Builder.CreateLoad(Builder.CreateStructGEP(frame_type[fun], sl, 0));
  }

  return std::make_pair(frame_type[fun], sl);
}

llvm::Value *IRGenerator::generate_vardecl(const VarDecl &decl)
{
  llvm::Value * alloc = nullptr;

  // if it does not escape
  if (!decl.get_escapes())
  {
    alloc = alloca_in_entry(llvm_type(decl.get_type()), std::string(decl.name));
    allocations[&decl]=alloc;
    return alloc;
  }
  else
  {
    std::vector<VarDecl *> decls = current_function_decl->get_escaping_decls();

    // find the position in the sequence of escaping variables
    auto it = std::find(decls.begin(), decls.end(), &decl);
    int pos = std::distance(decls.begin(), it);
    if (current_function_decl->get_parent()){
      pos = pos + 1;
    }
    frame_position[&decl] = pos;

    alloc = Builder.CreateStructGEP(frame_type[current_function_decl], frame, pos);
    allocations[&decl]=alloc;
    return alloc;
  }
}

} // namespace irgen
