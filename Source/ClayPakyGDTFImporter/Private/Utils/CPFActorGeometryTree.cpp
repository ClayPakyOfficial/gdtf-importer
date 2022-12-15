/*
MIT License

Copyright (c) 2022 Clay Paky S.P.A.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "Utils/CPFActorGeometryTree.h"
#include "ClayPakyGDTFImporterLog.h"
#include "CPGDTFImporterUtils.h"
#include "CPGDTFFixtureActor.h"
#include "ObjectTools.h"

#define LOCTEXT_NAMESPACE "CPFActorGeometryTree"

FActorGeometryTree::~FActorGeometryTree() {

	this->Components.Empty();
	this->Components.Shrink();
	this->BeamComponents.Empty();
	this->BeamComponents.Shrink();
	this->ParentActor = nullptr;
}

/**
 * Creates the object, all the SceneComponent tree and attach them to the Actor
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 13 June 2022
 *
 * @param Actor Actor to attach the components
 * @param FixturePackagePath
 * @param DMXModeIndex Index of the DMX Mode
 */
void FActorGeometryTree::CreateGeometryTree(ACPGDTFFixtureActor* Actor, FString FixturePackagePath, int DMXModeIndex) {

	if (Actor == nullptr || FixturePackagePath.IsEmpty()) return;
	if (DMXModeIndex < 0 || DMXModeIndex > Actor->GDTFDescription->GetDMXModes()->DMXModes.Num() - 1) {
		UE_LOG_CPGDTFIMPORTER(Warning, TEXT("Mode index out of bounds in '%s' on CreateGeometryTree"), *Actor->GetName());
		return;
	}

	this->ParentActor = Actor;
	this->NamePrefix = "";
	FName RootGeometryName = this->ParentActor->GDTFDescription->GetDMXModes()->DMXModes[DMXModeIndex].Geometry;
	UCPGDTFDescriptionGeometryBase* RootGeometry = nullptr;
	TArray<UCPGDTFDescriptionGeometryBase*> TopLevelGeometries = this->GetGDTFTopLevelGeometries();

	for (UCPGDTFDescriptionGeometryBase* Geometry : TopLevelGeometries) {

		if (Geometry->Name.ToString().Equals(RootGeometryName.ToString())) {
			RootGeometry = Geometry;
			break;
		}
	}

	if (RootGeometry == nullptr) {
		RootGeometry = TopLevelGeometries[0];
		UE_LOG_CPGDTFIMPORTER(Error, TEXT("Root geometry not found for this DMX mode. The behaviour may not be accurate"));
		FCPGDTFImporterUtils::SendNotification("Geometry error", "Root geometry not found for this DMX mode. The behaviour may not be accurate", SNotificationItem::CS_Fail);
	}

	USceneComponent* TreeBranch = this->CreateTreeBranch(this->ParentActor->GetRootComponent(), RootGeometry, Cast<UCPGDTFDescriptionModels>(Actor->GDTFDescription->Models), FixturePackagePath);
	TreeBranch->AttachToComponent(this->ParentActor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
}

/**
 * Clean the SceneComponent tree of the Actor
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 09 September 2022
 */
void FActorGeometryTree::DestroyGeometryTree() {

	this->DestroyTreeBranch(this->ParentActor->GetRootComponent());
	this->Components.Empty();
	this->Components.Shrink();
	this->BeamComponents.Empty();
	this->BeamComponents.Shrink();
}

/**
 * Fill the map with existing Actor Geometries
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 22 June 2022
 *
 * @param Actor Parent Actor
 */
void FActorGeometryTree::ReParseGeometryTree(ACPGDTFFixtureActor* Actor) {

	this->ParentActor = Actor;
	this->Components.Empty();
	this->BeamComponents.Empty();

	TArray<USceneComponent*> TopLevelComponents;
	Actor->GetRootComponent()->GetChildrenComponents(false, TopLevelComponents);

	for (USceneComponent* SubComponent : TopLevelComponents) {
		this->ParseTreeBranch(SubComponent);
	}
}

/**
 * Get all the beams under a given geometry name
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 27 June 2022
 *
 * @param GeometryName
 * @return Array of all beam subgeometries
 */
TArray<UCPGDTFBeamSceneComponent*> FActorGeometryTree::GetBeamsUnderGeometry(FName GeometryName) {
	
	return this->GetBeamsUnderGeometry_Internal(GeometryName);
}

/**
 * Get all the beams under a given geometry name
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 27 June 2022
 *
 * @param GeometryName
 * @param StartingNode Starting point for the search
 * @return Array of all beam subgeometries
 */
TArray<UCPGDTFBeamSceneComponent*> FActorGeometryTree::GetBeamsUnderGeometry_Internal(FName GeometryName, USceneComponent* StartingNode) {

	TArray<UCPGDTFBeamSceneComponent*> retSubBeams;

	TArray<USceneComponent*> Geometries;
	if (StartingNode == nullptr) {
		
		USceneComponent** StartingNodePtr;

		if (GeometryName.ToString().StartsWith(this->PREFIX_BEAM)) StartingNodePtr = (USceneComponent**) this->BeamComponents.Find(GeometryName);
		else StartingNodePtr = this->Components.Find(GeometryName);
		
		if (StartingNodePtr == nullptr) return retSubBeams;
		else StartingNode = *StartingNodePtr;
	}
	
	StartingNode->GetChildrenComponents(false, Geometries);

	for (USceneComponent* Geometry : Geometries) {

		TArray<UCPGDTFBeamSceneComponent*> SubBeams = this->GetBeamsUnderGeometry_Internal(GeometryName, Geometry);
		if (Geometry->GetName().StartsWith(this->PREFIX_BEAM, ESearchCase::CaseSensitive)) retSubBeams.Add(*this->BeamComponents.Find(FName(Geometry->GetName())));
		retSubBeams.Append(SubBeams);
	}
	return retSubBeams;
}

/**
 * Return the array of the first level of the geometry tree
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 09 September 2022
 *
 * @return TArray<UCPGDTFDescriptionGeometryBase*>
 */
TArray<UCPGDTFDescriptionGeometryBase*> FActorGeometryTree::GetGDTFTopLevelGeometries() {

	return Cast<UCPGDTFDescriptionGeometries>(this->ParentActor->GDTFDescription->Geometries)->Geometries;
}

/**
 * Creates a branch of the tree (the given geometry and all his childrens)
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 15 June 2022
 *
 * @param Parent
 * @param Geometry
 * @param Models
 * @param FixturePackagePath
 * @return Created branch
 */
USceneComponent* FActorGeometryTree::CreateTreeBranch(USceneComponent* Parent, UCPGDTFDescriptionGeometryBase* Geometry, UCPGDTFDescriptionModels* Models, FString FixturePackagePath) {

	// Creation of the container component
	USceneComponent* CurrentComponent = this->CreateAndAttachSceneComponent(Parent, Geometry->Name);

	// We set location and rotation
	FVector Location = Geometry->Position.GetColumn(3); // Location on last column
	FRotator ElementRotation;
	FCPGDTFImporterUtils::MatrixToRotator(Geometry->Position, &ElementRotation);
	CurrentComponent->SetRelativeLocationAndRotation(Location, ElementRotation);
	
	// Creation of the sub geometries
	for (UCPGDTFDescriptionGeometryBase* ChildGeometry : Geometry->Childrens) {
		this->CreateTreeBranch(CurrentComponent, ChildGeometry, Models, FixturePackagePath);
	}

	// Creation of StaticMesh
	if (!Geometry->Model.IsNone() && Geometry->Type != ECPGDTFDescriptionGeometryType::Beam) { // If the geometry is a beam the lens is managed by the Beam Component 
		if (!CreateStaticMeshComponentChild(CurrentComponent, Geometry, Models, FixturePackagePath)) {
			const FText Message = FText::Format(LOCTEXT("ImportFailed_Generic", "Error on '{0}' Actor creation. Unable to create {1} static mesh.\nPlease see Output Log for details."), FText::FromString(this->ParentActor->GetActorNameOrLabel()), FText::FromString(Geometry->Name.ToString()));
			FCPGDTFImporterUtils::SendNotification("Meshes import failed", Message.ToString(), SNotificationItem::CS_Fail);
			FMessageDialog::Open(EAppMsgType::Ok, Message);
		}
	}

	// Creation of Beam sub component
	if (Geometry->Type == ECPGDTFDescriptionGeometryType::Beam) {
		UCPGDTFDescriptionGeometryBeam* BeamGeometry = Cast<UCPGDTFDescriptionGeometryBeam>(Geometry);
		if (!CreateBeamComponentChild(CurrentComponent, BeamGeometry, Models, FixturePackagePath)) {
			const FText Message = FText::Format(LOCTEXT("ImportFailed_Generic", "Error on '{0}' Actor creation. Unable to create {1} Beam.\nPlease see Output Log for details."), FText::FromString(this->ParentActor->GetActorNameOrLabel()), FText::FromString(Geometry->Name.ToString()));
			FMessageDialog::Open(EAppMsgType::Ok, Message);
		}
	}

	if (Geometry->Type == ECPGDTFDescriptionGeometryType::GeometryReference) {
		UCPGDTFDescriptionGeometryReference* GeometryReference = Cast<UCPGDTFDescriptionGeometryReference>(Geometry);
		
		UCPGDTFDescriptionGeometryBase* ReferencedGeometry = nullptr;
		for (UCPGDTFDescriptionGeometryBase* TopLevelGeometry : this->GetGDTFTopLevelGeometries()) {

			if (GeometryReference->Geometry.ToString().Equals(TopLevelGeometry->Name.ToString())) {
				ReferencedGeometry = TopLevelGeometry;
				break;
			}
		}
		
		if (ReferencedGeometry != nullptr) {
			for (FDMXImportGDTFBreak Break : GeometryReference->Breaks) {

				this->NamePrefix = FString::FromInt(Break.DMXBreak).AppendChar('-').Append(FString::FromInt(Break.DMXOffset));
				this->NamePrefix.AppendChar('-');
				this->CreateTreeBranch(CurrentComponent, ReferencedGeometry, Models, FixturePackagePath);
			}
			this->NamePrefix = "";
		}
	}

	return CurrentComponent;
}

/**
 * Destroy a branch of the tree (All the childrens of the given USceneComponent)
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 09 September 2022
 *
 * @param Parent
 */
void FActorGeometryTree::DestroyTreeBranch(USceneComponent* Parent) {

	TArray<USceneComponent*> SubComponents;
	Parent->GetChildrenComponents(false, SubComponents);

	for (USceneComponent* SubComponent : SubComponents) {

		this->DestroyTreeBranch(SubComponent);
		FDetachmentTransformRules Rules = FDetachmentTransformRules(EDetachmentRule::KeepRelative, false);
		SubComponent->DetachFromComponent(Rules);
		this->ParentActor->RemoveInstanceComponent(SubComponent);
		SubComponent->DestroyComponent();
	}
}

/**
 * Parse a branch of the tree (the given geometry and all his childrens)
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 15 June 2022
 *
 * @param BranchRootComponent
 */
void FActorGeometryTree::ParseTreeBranch(USceneComponent* BranchRootComponent) {

	this->Components.Add(BranchRootComponent->GetFName(), BranchRootComponent);
	
	TArray<USceneComponent*> SubComponents;
	BranchRootComponent->GetChildrenComponents(false, SubComponents);

	for (USceneComponent* SubComponent : SubComponents) {

		// We register the Static Meshes
		if (SubComponent->GetName().Equals(FActorGeometryTree::PREFIX_STATIC_MESH + BranchRootComponent->GetFName().ToString())) {
			this->Components.Add(FName(SubComponent->GetName()), SubComponent);
			continue;
		}
		
		// We register the Beams
		else if (SubComponent->GetName().Equals(FActorGeometryTree::PREFIX_BEAM + BranchRootComponent->GetFName().ToString())) {
			this->BeamComponents.Add(FName(SubComponent->GetName()), Cast<UCPGDTFBeamSceneComponent>(SubComponent));
			continue;
		}

		this->ParseTreeBranch(SubComponent);
	}
}

/**
 * Creates a StaticMeshComponent and attach it to the parent.
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 13 June 2022
 *
 * @param Parent
 * @param Geometry
 * @param Models
 * @param FixturePackagePath
 * @return
 */
bool FActorGeometryTree::CreateStaticMeshComponentChild(USceneComponent* Parent, UCPGDTFDescriptionGeometryBase* Geometry, UCPGDTFDescriptionModels* Models, FString FixturePackagePath) {

	// Step 1: We select the good model
	FCPGDTFDescriptionModel Model;
	for (FCPGDTFDescriptionModel ModelCandidate : Models->Models) {
		if (ModelCandidate.Name.IsEqual(Geometry->Model)) {
			Model = ModelCandidate;
			break;
		}
	}
	if (!Model.Name.IsEqual(Geometry->Model)) {
		UE_LOG_CPGDTFIMPORTER(Error, TEXT("Unable to find model '%s' of Geometry '%s' during Actor construction"), *Geometry->Model.ToString(), *Geometry->Name.ToString());
		return false;
	}

	// Step 2: We load the Static Mesh
	UStaticMesh* StaticMesh = nullptr;
	if (Model.PrimitiveType == ECPGDTFDescriptionModelsPrimitiveType::Undefined) {
		TArray<UStaticMesh*> Meshes = FCPGDTFImporterUtils::LoadMeshesInFolder(FixturePackagePath + "/models/" + ObjectTools::SanitizeObjectName(Model.Name.ToString()));
		if (Meshes.Num() > 0) StaticMesh = Meshes[0];
	} else StaticMesh = FCPGDTFImporterUtils::LoadGDTFGenericMesh(Model.PrimitiveType);
	if (StaticMesh == nullptr) {
		UE_LOG_CPGDTFIMPORTER(Error, TEXT("Unable to load model '%s' of Geometry '%s' during Actor construction"), *Geometry->Model.ToString(), *Geometry->Name.ToString());
		return false;
	}

	// Step 3: Properties calcul
	FVector ActualSize = StaticMesh->GetBounds().GetBox().GetSize();
	FVector ScaleFactor = FVector(Model.Length, Model.Width, Model.Height) / ActualSize;

	// Step 4: We create the UStaticMeshComponent and attach it to his parent
	FName Name = FName(FActorGeometryTree::PREFIX_STATIC_MESH + this->NamePrefix + Geometry->Name.ToString());
	UStaticMeshComponent* Component = NewObject<UStaticMeshComponent>(this->ParentActor, UStaticMeshComponent::StaticClass(), Name);
	Component->SetStaticMesh(StaticMesh);
	Component->SetRelativeScale3D(ScaleFactor);
	Component->OnComponentCreated();
	Component->AttachToComponent(Parent, FAttachmentTransformRules::KeepRelativeTransform);
	this->ParentActor->AddInstanceComponent(Component);
	//Component->RegisterComponent(); // Disabled because of log warning
	return true;
}

/**
 * Creates a CPGDTFBeamSceneComponent and attach it to the parent.
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 15 June 2022
 *
 * @param Parent
 * @param Geometry
 * @param Models
 * @param FixturePackagePath
 * @return
 */
bool FActorGeometryTree::CreateBeamComponentChild(USceneComponent* Parent, UCPGDTFDescriptionGeometryBeam* Geometry, UCPGDTFDescriptionModels* Models, FString FixturePackagePath) {


	// We select the good model
	FCPGDTFDescriptionModel Model;
	for (FCPGDTFDescriptionModel ModelCandidate : Models->Models) {
		if (ModelCandidate.Name.IsEqual(Geometry->Model)) {
			Model = ModelCandidate;
			break;
		}
	}
	if (!Model.Name.IsEqual(Geometry->Model)) {
		UE_LOG_CPGDTFIMPORTER(Error, TEXT("Unable to find model '%s' of Geometry '%s' during Actor construction"), *Geometry->Model.ToString(), *Geometry->Name.ToString());
		return false;
	}

	FName Name = FName(FActorGeometryTree::PREFIX_BEAM + this->NamePrefix + Geometry->Name.ToString());
	UCPGDTFBeamSceneComponent* Component = NewObject<UCPGDTFBeamSceneComponent>(this->ParentActor, UCPGDTFBeamSceneComponent::StaticClass(), Name);
	Component->SetRelativeRotation(FRotator(-90, 0, 0));
	Component->PreConstruct(Geometry, &Model, FixturePackagePath);
	Component->OnComponentCreated();
	Component->AttachToComponent(Parent, FAttachmentTransformRules::KeepRelativeTransform);
	this->ParentActor->AddInstanceComponent(Component);
	//Component->RegisterComponent(); // Disabled because of log warning
	return true;
}

/**
 * Creates a USceneComponent and attach it to the parent.
 * @author Dorian Gardes - Clay Paky S.P.A.
 * @date 13 June 2022
 *
 * @param Parent Component to attach the newly created. If NULL we set the component at RootComponent
 * @param Name
 * @return Created Component
*/
USceneComponent* FActorGeometryTree::CreateAndAttachSceneComponent(USceneComponent* Parent, FName Name) {

	USceneComponent* NewComponent = NewObject<USceneComponent>(this->ParentActor, USceneComponent::StaticClass(), FName(this->NamePrefix + Name.ToString()));
	NewComponent->OnComponentCreated();
	if (Parent != nullptr) NewComponent->AttachToComponent(Parent, FAttachmentTransformRules::KeepRelativeTransform);
	this->ParentActor->AddInstanceComponent(NewComponent);
	//NewComponent->RegisterComponent(); // Disabled because of log warning
	return NewComponent;
}

#undef LOCTEXT_NAMESPACE