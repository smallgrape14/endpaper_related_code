#ifndef GET_ENTRYID_HPP
#define GET_ENTRYID_HPP

// #include "open.hpp"
#include "readdir.hpp"

#include <iostream>
#include <algorithm> // 为了使用std::fill
#include <vector>
#include <stdio.h>
#include <type_traits> // 引入 C++ 标准库中的 type_traits 头文件
#include <cstring>     // 引入 memcpy
// #include "change.hpp"
using namespace std;

// 函数将单个十六进制字符转换为对应的数值
int hexCharToValue(char c)
{
   LOG_printf(DEBUG, "char is %c \n", c);
   if (c >= '0' && c <= '9')
   {
      return c - '0';
   }
   else if (c >= 'a' && c <= 'f')
   {
      return 10 + c - 'a';
   }
   else if (c >= 'A' && c <= 'F')
   {
      return 10 + c - 'A';
   }
   else
   {
      LOG_printf(DEBUG, "非法字符 \n");
      return -1; // 非法字符
   }
}

// 函数将16进制字符串转换为数值数组
void hexStringTointArray(const char *hexStr, int *array)
{
   int len = strlen(hexStr);
   int cnt = 0;
   for (size_t i = 0; i < len; i += 2)
   {
      if (i + 1 < len)
      {
         int high = hexCharToValue(hexStr[i]);
         int low = hexCharToValue(hexStr[i + 1]);

         if (high == -1 || low == -1)
         {
            LOG_printf(DEBUG, "Error: Invalid hex character found\n");
            return; // 包含错误处理
         }

         int value = (high << 4) | low; // 组合高4位和低4位
         // LOG_printf(DEBUG,"0x%01X%01X\n", high, low);
         // LOG_printf(DEBUG,"%02X\n", value);

         array[cnt] = value;
         cnt++;
      }
   }
}

int test_open(int input)
{
   LOG(DEBUG) << "input =" << input << "\n";
   return input + 10;
}
// 第一个参数 Path 和第二个参数 路径深度pathdepth ---------------------------------------------------------------------------------

// 参数1  传入参数 -----------------------------------------------------------------begin
// 运行会报这个错误ERROR：
/*
   free(): double free detected in tcache 2
   Aborted (core dumped)
   原因是 ： FhgfsOpsErr findOwner() 忘记return 了
*/
class Path
{
public:
   Path() {}

   // 构造函数，初始化操作
   // explicit关键字表示这个构造函数是显式的，这意味着它禁止了从单个参数到Path对象的隐式转换。
   // std::move(str)是一个标准库算法，它将str转换为右值引用，这样做可以避免复制字符串，因为构造函数接收的是一个临时对象。这是移动语义的一个应用，可以提高效率。
   // explicit Path(std::string str) :
   explicit Path(std::string str) : pathStr(std::move(str))
   {
      /*
         更新dirSeparators成员变量，它通常是一个存储目录分隔符位置的std::vector。
         调用updateDirSeparators()是为了在创建Path对象时立即解析路径字符串，并准备好后续可能对路径进行的操作，如分割路径、获取目录名等。
      */
      updateDirSeparators();
      LOG(DEBUG) << "---------end -------\n";
   }
   ~Path()
   {
   }

   bool absolute() const
   {
      return !dirSeparators.empty() && dirSeparators.front() == 0;
   }

   // @returns the number of components in the path.

   size_t size() const
   {
      if (pathStr.empty())
         return 0;
      else if (absolute())
         return dirSeparators.size();
      else
         return dirSeparators.size() + 1;
   }

   // @returns the last element of the path.

   std::string back() const
   {
      if (dirSeparators.empty())
         return pathStr;
      else
         return pathStr.substr(dirSeparators.back() + 1);
   }

   // @returns the first component of the path.

   std::string front() const
   {
      if (dirSeparators.empty())
         return pathStr;
      else if (!absolute())
         return pathStr.substr(0, dirSeparators[0]);
      else if (dirSeparators.size() == 1)
         return pathStr.substr(1);
      else
         return pathStr.substr(1, dirSeparators[1] - 1);
   }

   std::string getpathStr()
   {
      return this->pathStr;
   }
   std::vector<std::string::size_type> getdirSeparators()
   {
      return this->dirSeparators;
   }
   // @returns the index'th element of the Path.
   // @throws std::out_of_range if index is out of range (i.e. index >= this->size()).

   std::string operator[](size_t index) const
   {
      if (absolute())
      {
         if (index >= dirSeparators.size())
            throw std::out_of_range("Path element index out of range.");

         const std::string::size_type left = dirSeparators[index] + 1;

         if (index == dirSeparators.size() - 1)
            return pathStr.substr(left);

         const std::string::size_type right = dirSeparators[index + 1] - left;

         return pathStr.substr(left, right);
      }
      else
      {
         if (index > dirSeparators.size())
            throw std::out_of_range("Path element index out of range.");

         if (index == 0)
         {
            if (dirSeparators.empty())
               return pathStr;
            else
               return pathStr.substr(0, dirSeparators[0]);
         }

         const std::string::size_type left = dirSeparators[index - 1] + 1;

         if (index == dirSeparators.size())
            return pathStr.substr(left);

         const std::string::size_type right = dirSeparators[index] - left;

         return pathStr.substr(left, right);
      }
   }

   /*
         void operator=(const std::string& str)
         {
            pathStr = str;
            updateDirSeparators();
         }

         const std::string& str() const
         {
            return pathStr;
         }







         bool empty() const
         {
            return pathStr.empty();
         }

         // @returns the dirname of the path, i.e. the whole path excluding the last component.

         Path dirname() const
         {
            if (dirSeparators.size() > 0)
               return Path(pathStr.substr(0, dirSeparators.back()));
            else
               return Path(".");
         }





         // Concatenates a path to the existing path.
         //@throws std::invalid_argument if path to be concatenated is absolute.

         Path& operator/=(const Path& rhs)
         {
            if (rhs.absolute())
            {
   #ifdef BEEGFS_DEBUG
               LogContext(__func__).logBacktrace();
   #endif
               throw std::invalid_argument("Can't concatenate absolute path; "
                     "Base: " + pathStr + " Concatenated path: " + rhs.pathStr + ".");
            }

            if (empty())
            {
               *this = rhs;
               return *this;
            }

            if (!rhs.empty())
            {
               dirSeparators.push_back(pathStr.length());

               for (auto it = rhs.dirSeparators.begin(); it != rhs.dirSeparators.end(); ++it)
                  dirSeparators.push_back(*it + pathStr.length() + 1);

               pathStr.reserve(pathStr.length() + 1 + rhs.pathStr.length());
               pathStr.append("/");
               pathStr.append(rhs.pathStr);
            }

            return *this;
         }

         Path& operator/=(const char* rhs)
         {
            if (empty())
            {
               *this = rhs;
               return *this;
            }

            const size_t rhsLen = std::strlen(rhs);
            if (rhsLen != 0)
            {
               if (rhs[0] == '/')
               {
   #ifdef BEEGFS_DEBUG
               LogContext(__func__).logBacktrace();
   #endif
                  throw std::invalid_argument("Can't concatenate absolute path; "
                        "Base: " + pathStr + " Concatenated path: " + rhs + ".");
               }

               dirSeparators.push_back(pathStr.length());

               for (size_t i = 0; i < rhsLen; i++)
                  if (rhs[i] == '/')
                     dirSeparators.push_back(pathStr.length() + i + 1);

               pathStr.reserve(pathStr.length() + 1 + rhsLen);
               pathStr.append("/");
               pathStr.append(rhs);
            }

            return *this;
         }

         Path& operator/=(const std::string& rhs)
         {
            return operator/=(rhs.c_str());
         }

         friend Path operator/(const Path& lhs, const Path& rhs)
         {
            Path res(lhs);
            res /= rhs;
            return res;
         }

         friend Path operator/(const Path& lhs, const std::string& rhs)
         {
            Path res(lhs);
            res /= rhs;
            return res;
         }

         friend Path operator/(const std::string& lhs, const Path& rhs)
         {
            Path res(lhs);
            res /= rhs;
            return res;
         }

         friend Path operator/(const Path& lhs, const char* rhs)
         {
            Path res(lhs);
            res /= rhs;
            return res;
         }

         friend Path operator/(const char* lhs, const Path& rhs)
         {
            Path res(lhs);
            res /= rhs;
            return res;
         }

         friend bool operator==(const Path& lhs, const Path& rhs)
         {
            // No need to compare dirSeparators here
            return lhs.pathStr == rhs.pathStr;
         }

         friend bool operator!=(const Path& lhs, const Path& rhs)
         {
            return lhs.pathStr != rhs.pathStr;
         }

         static void serialize(const Path* obj, Serializer& ser)
         {
            ser % obj->pathStr;
         }

         static void serialize(Path* obj, Deserializer& des)
         {
            des % obj->pathStr;
            obj->updateDirSeparators();
         }

         friend std::ostream& operator<<(std::ostream& ostr, const Path& p)
         {
            ostr << p.pathStr;
            return ostr;
         }
   */
private:
   std::string pathStr;
   /*
      变量1：std::vector：一个容器类，用于存储元素集合。std::vector 提供了动态数组的功能，可以自动调整大小以容纳更多元素。它支持在末端快速插入和删除元素，以及随机访问元素。
      变量2：std::string::size_type：这是 std::string 类的一个成员类型，通常是一个无符号整数类型，用于表示字符串中的位置或长度。在这个上下文中，std::string::size_type 用作 std::vector 的元素类型，这意味着 dirSeparators 将存储与字符串相关的大小或位置信息。
      变量3：dirSeparators 用于存储字符串中分隔符的位置索引。
         在文件路径处理的上下文中，dirSeparators 可能用于存储一个文件路径字符串中每个目录分隔符（如 / 或 \）的位置。
         例如，在 Unix 和 Linux 系统中，文件路径使用 / 作为分隔符，而在 Windows 系统中，路径可能使用 \ 作为分隔符。
         通过存储这些分隔符的位置，可以方便地解析路径中的各个目录和文件名。
   */
   std::vector<std::string::size_type> dirSeparators;

   /**
    * Updates the dirSeparators vector and compresses any consecutive directory separators
    * contained in the path string.
    * 更新类的成员变量dirSeparators，该变量是一个存储目录分隔符位置的std::vector。
    * //同时，这个函数还负责压缩路径字符串中连续的目录分隔符，并处理路径字符串尾部的分隔符
    */
   void updateDirSeparators()
   {
      // inputIndex用于跟踪原始路径字符串pathStr中的位置。
      // outputIndex用于跟踪压缩后的路径字符串中的位置。
      std::string::size_type inputIndex = 0;
      std::string::size_type outputIndex = 0;

      // Compress consecutive slashes
      // 压缩连续的分隔符：
      LOG(DEBUG) << "pathStr.size() = " << pathStr.size() << "\n";
      while (inputIndex < pathStr.size())
      {
         pathStr[outputIndex++] = pathStr[inputIndex++];
         // 如果当前字符是分隔符'/'，则将其添加到压缩后的路径字符串中，并且记录它的位置到dirSeparators向量中。
         if (pathStr[inputIndex - 1] == '/')
         {
            dirSeparators.push_back(outputIndex - 1);
            // 如果存在连续的分隔符，inputIndex会跳过这些连续的分隔符，只将第一个分隔符添加到pathStr中。
            while (pathStr[inputIndex] == '/')
               inputIndex++;
         }
      }

      // Remove trailing slash if there is one
      // 如果压缩后的路径字符串不为空，并且最后一个字符是分隔符，则移除它。
      if (!pathStr.empty() && pathStr[outputIndex - 1] == '/')
      {
         pathStr.resize(outputIndex - 1);
         // 如果移除了尾部分隔符，那么dirSeparators向量中最后一个元素（即尾部分隔符的位置）也应该被移除。
         dirSeparators.pop_back();
      }
      else // 如果没有移除尾部分隔符，那么调整pathStr的大小以匹配outputIndex。
      {
         pathStr.resize(outputIndex);
      }
      LOG(DEBUG) << "^^^^pathStr = " << pathStr << "\n";
      LOG(DEBUG) << "dirSeparators.size() = " << dirSeparators.size() << "\n";
      for (int i = 0; i < dirSeparators.size(); i++)
      {
         LOG(DEBUG) << "-- " << i << " -- = " << dirSeparators[i] << "\t,";
      }
      LOG(DEBUG) << "\n";
   }
};

// 参数1  传入参数 -----------------------------------------------------------------end

// 参数 currentNode --------------------------------------------------------------begin

// 参数 currentNode metaNode -----------------------------------------------------------------------------
/**
 * This class represents a metadata, storage, client, etc node (aka service). It contains things
 * like ID and feature flags of a node and also provides the connection pool to communicate with
 * that node.
 */
// 注意 ：NOTE: typedef NumericID<uint32_t, NumNodeIDTag> NumNodeID 这个变量被我改成一个简单的结构体了，可能会影响这里的类里面的定义
class Node_class
{
public:
   //  Node(NodeType nodeType, std::string nodeID, NumNodeID nodeNumID, unsigned short portUDP,
   //       unsigned short portTCP, const NicAddressList& nicList);
   Node_class(NodeType nodeType, std::string nodeID, NumNodeID nodeNumID, unsigned short portUDP);
   virtual ~Node_class();

   /*
      Node(const Node&) = delete;
      Node& operator=(const Node&) = delete;

      // disabled only because it is not needed at the moment.
      Node(Node&&) = delete;
      Node& operator=(Node&&) = delete;

      void updateLastHeartbeatT();
      Time getLastHeartbeatT();
      bool updateInterfaces(unsigned short portUDP, unsigned short portTCP,
         NicAddressList& nicList);

      std::string getTypedNodeID() const;
      std::string getNodeIDWithTypeStr() const;

      // static
      static std::string getTypedNodeID(std::string nodeID, NumNodeID nodeNumID, NodeType nodeType);
      static std::string getNodeIDWithTypeStr(std::string nodeID, NumNodeID nodeNumID,
         NodeType nodeType);

   */
protected:
   NodeType nodeType;

   /*
      Node(NodeType nodeType, std::string nodeID, NumNodeID nodeNumID, unsigned short portUDP);
      Mutex mutex;
      void updateLastHeartbeatTUnlocked();
      Time getLastHeartbeatTUnlocked();
      bool updateInterfacesUnlocked(unsigned short portUDP, unsigned short portTCP,
         NicAddressList& nicList);

      // getters & setters
      void setConnPool(NodeConnPool* connPool)
      {
         this->connPool = connPool;
      }
   */

private:
   std::string id;  // string ID, generated locally on each node
   NumNodeID numID; // numeric ID, assigned by mgmtd server store
   unsigned short portUDP;

   /*
      NodeConnPool* connPool;


      Time lastHeartbeatT; // last heartbeat receive time
   */

public:
   // getters & setters

   const std::string &getID()
   {
      return id;
   }

   NumNodeID getNumID() const
   {
      return numID;
   }

   void setNumID(NumNodeID numID)
   {
      this->numID = numID;
   }

   NodeType getNodeType() const
   {
      return nodeType;
   }

   /*
      NicAddressList getNicList()
      {
         return connPool->getNicList();
      }

      NodeConnPool* getConnPool() const
      {
         return connPool;
      }

      unsigned short getPortUDP()
      {
         const std::lock_guard<Mutex> lock(mutex);

         return this->portUDP;
      }

      virtual unsigned short getPortTCP()
      {
         return this->connPool->getStreamPort();
      }




      struct VectorDes
      {
         bool v6;
         std::vector<NodeHandle>& nodes;

         Deserializer& runV6(Deserializer& des) const
         {
            uint32_t elemCount;
            uint32_t padding;

            des
               % elemCount
               % padding; // PADDING

            if(!des.good())
               return des;

            nodes.clear();

            while (nodes.size() != elemCount)
            {
               // PADDING
               PadFieldTo<Deserializer> pad(des, 8);

               NicAddressList nicList;
               char nodeType = 0;
               unsigned fhgfsVersion = 0;
               BitStore nodeFeatureFlags;
               uint16_t portTCP = 0;
               uint16_t portUDP = 0;
               NumNodeID nodeNumID;
               std::string nodeID;

               des
                  % nodeFeatureFlags
                  % serdes::stringAlign4(nodeID)
                  % nicList
                  % fhgfsVersion
                  % nodeNumID
                  % portUDP
                  % portTCP
                  % nodeType;

               if(unlikely(!des.good()))
                  break;

               nodes.push_back(std::make_shared<Node>(NodeType(nodeType), nodeID, nodeNumID,
                     portUDP, portTCP, nicList));
            }

            return des;
         }

         Deserializer& run(Deserializer& des) const
         {
            uint32_t elemCount;

            des % elemCount;

            if(!des.good())
               return des;

            nodes.clear();

            while (nodes.size() != elemCount)
            {
               NicAddressList nicList;
               char nodeType = 0;
               uint16_t portTCP = 0;
               uint16_t portUDP = 0;
               NumNodeID nodeNumID;
               std::string nodeID;

               des
                  % nodeID
                  % nicList
                  % nodeNumID
                  % portUDP
                  % portTCP
                  % nodeType;

               if(unlikely(!des.good()))
                  break;

               nodes.push_back(std::make_shared<Node>(NodeType(nodeType), nodeID, nodeNumID,
                        portUDP, portTCP, nicList));
            }

            return des;
         }

         friend Deserializer& operator%(Deserializer& des, const VectorDes& value)
         {
            if (value.v6)
               return value.runV6(des);
            else
               return value.run(des);
         }
      };

      static VectorDes inV6Format(std::vector<NodeHandle>& nodes)
      {
         return {true, nodes};
      }
   */
};

/**
 * Constructor for derived classes that provide their own connPool, e.g. LocalNode.
 *
 * Note: derived classes: do not forget to set the connPool!
 *
 * @param portUDP value 0 if undefined
 */

Node_class::Node_class(NodeType nodeType, std::string nodeID, NumNodeID nodeNumID, unsigned short portUDP) : nodeType(nodeType), id(std::move(nodeID))
{
   this->numID = nodeNumID;
   this->portUDP = portUDP;

   // derived classes: do not forget to set the connPool!
}

Node_class::~Node_class()
{
   // SAFE_DELETE_NOSET(connPool);
}

/*
template <typename T, typename Tag>
class NumericID final
{
   public:
      typedef T ValueT;

      NumericID():
         value(0) { }

      explicit NumericID(T value):
         value(value) { }

      T val() const
      {
         return value;
      }

      std::string str() const
      {
         std::stringstream out;
         out << value;
         return out.str();
      }

      std::string strHex() const
      {
         std::stringstream out;
         out << std::hex << std::uppercase << value;
         return out.str();
      }

      void fromHexStr(const std::string& valueStr)
      {
         std::stringstream in(valueStr);
         in >> std::hex >> value;
      }

      void fromStr(const std::string& valueStr)
      {
         std::stringstream in(valueStr);
         in >> value;
      }

      template<typename This, typename Ctx>
      static void serialize(This obj, Ctx& ctx)
      {
         ctx % obj->value;
      }

      bool operator==(const NumericID& other) const
      {
         return (value == other.value);
      }

      bool operator!=(const NumericID& other) const
      {
         return (value != other.value);
      }

      bool operator<(const NumericID& other) const
      {
         return (value < other.value);
      }

      bool operator>(const NumericID& other) const
      {
         return (value > other.value);
      }

      bool operator<=(const NumericID& other) const
      {
         return (value <= other.value);
      }

      bool operator>=(const NumericID& other) const
      {
         return (value >= other.value);
      }

      NumericID& operator++()
      {
         ++value;
         return *this;
      }

      NumericID operator++(int)
      {
         NumericID result = *this;
         ++value;
         return result;
      }

      NumericID& operator--()
      {
         --value;
         return *this;
      }

      NumericID operator--(int)
      {
         NumericID result = *this;
         --value;
         return result;
      }

      bool operator!() const
      {
         return (value == 0);
      }

      explicit operator bool() const
      {
         return value != 0;
      }

      friend std::ostream& operator<< (std::ostream& out, const NumericID& obj)
      {
         return out << obj.str();
      }

      friend std::istream& operator>> (std::istream& in, NumericID& obj)
      {
         in >> obj.value;
         return in;
      }

   protected:
      T value;
};

enum NumNodeIDTag {};
typedef NumericID<uint32_t, NumNodeIDTag> NumNodeID;//@@有一个同名的结构体 这个我直接注释了

// typedef std::map<NumNodeID, std::shared_ptr<Node>> NodeMap;

*/

// static std::shared_ptr<Node> referenceRoot(NodeStoreServers& nodes, const RootInfo& root,MirrorBuddyGroupMapper& bgm)
static std::shared_ptr<Node_class> referenceRoot()
{
   uint32_t metaNodeID = 1; //@@根节点所在的元数据节点ID  224 server :190 162server :1
                            //  NumNodeID root(metaNodeID);
   NumNodeID root;
   root.value = metaNodeID;

   // const auto id =root.getID();
   const auto id = root;

   const auto isMirrored = false; //@@ //root.getIsMirrored();
                                  // if (!isMirrored)
                                  //    return nodes.referenceNode(id);

   // return nodes.referenceNode(NumNodeID(bgm.getPrimaryTargetID(id.val())));
   // uint16_t PrimaryTargetID=root.val();//@@元数据节点的主NodeID
   uint16_t PrimaryTargetID = root.value; //@@元数据节点的主NodeID 这个变量也没有用到

   std::string nodeID = "ubuntu-PowerEdge-R750xa"; //@@ 224: xfusion3; 217 dell01 ; 162:
   unsigned short portUDP = 8005;                  //@@ meta:8005 storage:8003

   // 使用 make_shared 初始化 std::shared_ptr<Node>
   auto metanode = std::make_shared<Node_class>(NODETYPE_Meta, nodeID, root, portUDP);
   return metanode;
}

// 参数 currentNode --------------------------------------------------------------end

// 参数3 EntryInfo_class 类 -----------------------------------------------------------------------------begin

/**
 * Information about a file/directory
 */
class EntryInfo_class
{
public:
   EntryInfo_class()
       //  : ownerNodeID(0), entryType(DirEntryType_INVALID), featureFlags(0)
       : entryType(DirEntryType_INVALID), featureFlags(0)
   {
      ownerNodeID.value = 0;
   }

   EntryInfo_class(const NumNodeID ownerNodeID, const std::string &parentEntryID,
                   const std::string &entryID, const std::string &fileName, const DirEntryType entryType,
                   const int featureFlags) : ownerNodeID(ownerNodeID),
                                             parentEntryID(parentEntryID),
                                             entryID(entryID),
                                             fileName(fileName),
                                             entryType(entryType),
                                             featureFlags(featureFlags)
   {
      LOG(DEBUG) << "EntryInfo_class 构造函数： ownerNodeID= " << ownerNodeID.value << "\t"
                 << "parentEntryID=" << parentEntryID << "\t , entryID =" << entryID << "\t, fileName ="
                 << fileName << "\t , \n";
   }

   virtual ~EntryInfo_class()
   {
   }

   bool operator==(const EntryInfo_class &other) const;

   bool operator!=(const EntryInfo_class &other) const { return !(*this == other); }
   // template<typename This, typename Ctx>
   //    static void serialize(This obj, Ctx& ctx)
   //    {
   //       uint16_t padding = 0; // PADDING

   //       ctx
   //          % serdes::as<uint32_t>(obj->entryType)
   //          % obj->featureFlags
   //          % serdes::stringAlign4(obj->parentEntryID)
   //          % serdes::stringAlign4(obj->entryID)
   //          % serdes::stringAlign4(obj->fileName)
   //          % obj->ownerNodeID
   //          % padding;
   //    }
protected:
   NumNodeID ownerNodeID;     // nodeID of the metadata server that has the inode
   std::string parentEntryID; // entryID of the parent dir
   std::string entryID;       // entryID of the actual entry
   std::string fileName;      // file/dir name of the actual entry

   DirEntryType entryType;
   int32_t featureFlags; // feature flags (e.g. ENTRYINFO_FEATURE_INLINED)

public:
   void set(const EntryInfo_class *newEntryInfo)
   {
      this->ownerNodeID = newEntryInfo->getOwnerNodeID();
      this->parentEntryID = newEntryInfo->getParentEntryID();
      this->entryID = newEntryInfo->getEntryID();
      this->fileName = newEntryInfo->getFileName();
      this->entryType = newEntryInfo->getEntryType();
      this->featureFlags = newEntryInfo->getFeatureFlags();
   }
   NumNodeID getOwnerNodeID() const
   {
      return this->ownerNodeID;
   }

   const std::string &getParentEntryID() const
   {
      return this->parentEntryID;
   }

   const std::string &getEntryID() const
   {
      return this->entryID;
   }

   const std::string &getFileName() const
   {
      return this->fileName;
   }

   DirEntryType getEntryType() const
   {
      return this->entryType;
   }

   int getFeatureFlags() const
   {
      return this->featureFlags;
   }
   // inliners

   void set(const NumNodeID ownerNodeID, const std::string &parentEntryID,
            const std::string &entryID, const std::string &fileName,
            const DirEntryType entryType, int featureFlags)
   {
      this->ownerNodeID = ownerNodeID;
      this->parentEntryID = parentEntryID;
      this->entryID = entryID;
      this->fileName = fileName;
      this->entryType = entryType;
      this->featureFlags = featureFlags;
   }
   /*
         bool getIsInlined() const
         {
            return (this->featureFlags & ENTRYINFO_FEATURE_INLINED) ? true : false;
         }

         bool getIsBuddyMirrored() const
         {
            return (this->featureFlags & ENTRYINFO_FEATURE_BUDDYMIRRORED) ? true : false; //0 & 10
         }
   */

   /*







      void setParentEntryID(const std::string& newParentEntryID)
      {
         this->parentEntryID = newParentEntryID;
      }

      // Set or unset the ENTRYINFO_FEATURE_INLINED flag.

      void setInodeInlinedFlag(const bool isInlined)
      {
         if (isInlined)
            this->featureFlags |= ENTRYINFO_FEATURE_INLINED;
         else
            this->featureFlags &= ~(ENTRYINFO_FEATURE_INLINED);
      }

      void setBuddyMirroredFlag(const bool isBuddyMirrored)
      {
         if (isBuddyMirrored)
            this->featureFlags |= ENTRYINFO_FEATURE_BUDDYMIRRORED;
         else
            this->featureFlags &= ~(ENTRYINFO_FEATURE_BUDDYMIRRORED);
      }


      */
};

// 参数3 entryinfo 类 -----------------------------------------------------------------------------end

// 参数41 FindOwnerMsg---------------------------------------------------------------begin

// FindOwnerMsg requestMsg                 --------------------------------------------------------------

// namespace serdes {  //@@ 我注释的

//    template<typename Object>
//    inline BackedPtrDes<Object, std::unique_ptr<Object>>
//       backedPtr(Object*& ptr, std::unique_ptr<Object>& backing)
//    {
//       return BackedPtrDes<Object, std::unique_ptr<Object>>(ptr, backing);
//    }

// }

#define NETMSGTYPE_FindOwner 2035

// 参数41 FindOwnerMsg ---------------------------------------------------------------end

// 参数42 EntryInfoWithDepth---------------------------------------------------------------begin
// EntryInfoWithDepth entryInfoWDepth;  --------------------------------------------------------------------

/**
 */
class EntryInfoWithDepth : public EntryInfo_class
{
public:
   EntryInfoWithDepth() : EntryInfo_class(), entryDepth(0)
   {
   }

   EntryInfoWithDepth(const NumNodeID ownerNodeID, const std::string &parentEntryID,
                      const std::string &entryID, const std::string &fileName, const DirEntryType entryType,
                      const int featureFlags, const unsigned entryDepth) : EntryInfo_class(ownerNodeID, parentEntryID, entryID, fileName, entryType, featureFlags),
                                                                           entryDepth(entryDepth)
   {
   }

   EntryInfoWithDepth(const EntryInfo_class &entryInfo) : EntryInfo_class(entryInfo),
                                                          entryDepth(0)
   {
   }

   virtual ~EntryInfoWithDepth()
   {
   }

   // template<typename This, typename Ctx>
   // static void serialize(This obj, Ctx& ctx)
   // {
   //    ctx
   //       % serdes::base<EntryInfo_class>(obj)
   //       % obj->entryDepth;
   // }

private:
   uint32_t entryDepth; // 0-based path depth (incl. root dir)

public:
   // inliners
   void set(const NumNodeID ownerNodeID, const std::string &parentEntryID,
            const std::string &entryID, const std::string &fileName, const DirEntryType entryType,
            const int featureFlags, const unsigned entryDepth)
   {
      EntryInfo_class::set(ownerNodeID, parentEntryID, entryID, fileName, entryType, featureFlags);

      this->entryDepth = entryDepth;
   }

   unsigned getEntryDepth() const
   {
      return entryDepth;
   }

   void setEntryDepth(const unsigned entryDepth)
   {
      this->entryDepth = entryDepth;
   }
   // 获取 EntryInfo_class 的成员变量
   void getEntryInfo(NumNodeID &outOwnerNodeID, char *outParentEntryID,
                     char *outEntryID, char *outFileName,
                     DirEntryType &outEntryType, int &outFeatureFlags) const
   {
      outOwnerNodeID = getOwnerNodeID();
      //   outParentEntryID = getParentEntryID();
      //   outEntryID = getEntryID();
      //   outFileName = getFileName();
      strcpy(outParentEntryID, getParentEntryID().c_str());
      strcpy(outEntryID, getEntryID().c_str());
      strcpy(outFileName, getFileName().c_str());

      outEntryType = getEntryType();
      outFeatureFlags = getFeatureFlags();
      LOG(DEBUG) << "--------- print out Entryinfo ----------- \n";
      //   LOG(DEBUG) << "Owner Node ID: " << outOwnerNodeID.value << std::endl;
      LOG(DEBUG) << "Parent Entry ID: " << outParentEntryID << std::endl;
      LOG(DEBUG) << "Entry ID: " << outEntryID << std::endl;
      LOG(DEBUG) << "File Name: " << outFileName << std::endl;
      //   LOG(DEBUG) << "Entry Type: " << static_cast<int>(outEntryType) << std::endl;
      //   LOG(DEBUG) << "Entry Type: " << (outEntryType) << std::endl;

      //   LOG(DEBUG) << "Feature Flags: " << outFeatureFlags << std::endl;
      LOG(DEBUG) << "--------- ----------- ----------- \n";
   }

   //  void getEntryInfo(NumNodeID& outOwnerNodeID, std::string& outParentEntryID,
   //                    std::string& outEntryID, std::string& outFileName,
   //                    DirEntryType& outEntryType, int& outFeatureFlags) const {
   //      outOwnerNodeID = getOwnerNodeID();
   //      outParentEntryID = getParentEntryID();
   //      outEntryID = getEntryID();
   //      outFileName = getFileName();
   //      outEntryType = getEntryType();
   //      outFeatureFlags = getFeatureFlags();
   //    //   LOG(DEBUG)<<"--------- print out Entryinfo ----------- \n";
   //    //   LOG(DEBUG) << "Owner Node ID: " << outOwnerNodeID.value << std::endl;
   //    //   LOG(DEBUG) << "Parent Entry ID: " << outParentEntryID << std::endl;
   //    //   LOG(DEBUG) << "Entry ID: " << outEntryID << std::endl;
   //    //   LOG(DEBUG) << "File Name: " << outFileName << std::endl;
   //    // //   LOG(DEBUG) << "Entry Type: " << static_cast<int>(outEntryType) << std::endl;
   //    //   LOG(DEBUG) << "Entry Type: " << (outEntryType) << std::endl;

   //    //   LOG(DEBUG) << "Feature Flags: " << outFeatureFlags << std::endl;
   //    //   LOG(DEBUG)<<"--------- ----------- ----------- \n";

   //  }
};

// 参数42 EntryInfoWithDepth---------------------------------------------------------------end

// 参数43 findOwnerStep ---------------------------------------------------------------begin

#define NETMSGTYPE_FindOwnerResp 2036

class NetMessage_class;

/**
 * Arguments for request-response communication.
 * Note: Destructor will free response msg and response buf, if they are not NULL.
 */
struct RequestResponseArgs
{
   /**
    * @param node may be NULL, depending on which requestResponse...() method is used.
    * @param respMsgType expected type of response message (NETMSGTYPE_...)
    */
   RequestResponseArgs(const Node_class *node, NetMessage_class *requestMsg, unsigned respMsgType,
                       FhgfsOpsErr (*sendExtraData)(Socket *, void *) = nullptr, void *extraDataContext = nullptr)
       : node(node), requestMsg(requestMsg), respMsgType(respMsgType),
         logFlags(0), minTimeoutMS(0), sendExtraData(sendExtraData),
         extraDataContext(extraDataContext)
   {
      // see initializer list
   }

   const Node_class *node; // receiver

   NetMessage_class *requestMsg;
   unsigned respMsgType; // expected type of response message (NETMSGTYPE_...)

   std::unique_ptr<NetMessage_class> outRespMsg; // received response message

   // internal (initialized by MessagingTk_requestResponseWithRRArgs() )
   unsigned char logFlags; // REQUESTRESPONSEARGS_LOGFLAG_... combination to avoid double-logging

   int minTimeoutMS; // minimum communication timeout. negative values disable timeouts.

   // hook to send extra data after the message
   FhgfsOpsErr (*sendExtraData)(Socket *, void *);
   void *extraDataContext;
};

#define MSGBUF_DEFAULT_SIZE (64 * 1024)   // at least big enough to store a datagram
#define MSGBUF_MAX_SIZE (4 * 1024 * 1024) // max accepted size

//----------------------------tmp_createmsgvec.cpp -----------------begin
class Serializer;
#define NETMSG_DEFAULT_USERID (~0) // non-zero to avoid mixing up with root userID

class Deserializer
{
public:
   Deserializer(const char *buffer, unsigned bufferSize)
       : buffer(buffer), bufferSize(bufferSize), bufferOffset(0)
   {
   }

   Deserializer(Deserializer &&other)
       : buffer(nullptr), bufferSize(-1), bufferOffset(0)
   {
      swap(other);
   }

   Deserializer &operator=(Deserializer &&other)
   {
      Deserializer(std::move(other)).swap(*this);
      return *this;
   }

private:
   const char *buffer;
   unsigned bufferSize;
   unsigned bufferOffset;

   // template<typename Value, void (*Fn)(Value*, Deserializer&)>
   // struct has_deserializer : boost::true_type {};//@@

   Deserializer(char *buffer, unsigned bufferSize, unsigned bufferOffset)
       : buffer(buffer), bufferSize(bufferSize), bufferOffset(bufferOffset)
   {
   }

public:
   bool isReading() const { return true; }
   bool isWriting() const { return false; }

   const char *currentPosition() const
   {
      return this->buffer + this->bufferOffset;
   }

   unsigned size() const
   {
      return this->bufferOffset;
   }

   bool good() const
   {
      return this->bufferSize && this->bufferOffset <= this->bufferSize;
   }

   void swap(Deserializer &other)
   {
      std::swap(buffer, other.buffer);
      std::swap(bufferSize, other.bufferSize);
      std::swap(bufferOffset, other.bufferOffset);
   }

   void setBad()
   {
      this->bufferSize = 0;
   }
   // 重载运算符%以反序列化NetMessageHeader类型的数据
   Deserializer &operator%(NetMessageHeader &msgHeader)
   { //$$也要重写

      DeserializeCtx ctx = {
          .data = buffer,
          .length = bufferSize, //$$
      };
      LOG(DEBUG) << "before ctx.length= " << ctx.length << "\n";

      __NetMessage_deserializeHeader(&ctx, &msgHeader);
      LOG(DEBUG) << "after ctx.length= " << ctx.length << "\n";
      return *this;
   }

   Deserializer &operator%(uint32_t &value)
   {
      DeserializeCtx ctx = {
          .data = buffer,
          .length = sizeof(value)};
      LOG(DEBUG) << "deserializer \n";
      for (int i = 0; i < 4; i++)
      {
         LOG(DEBUG) << buffer[i] << " ";
      }
      LOG(DEBUG) << "value = " << value << "\n";
      Serialization_deserializeUInt(&ctx, &value);
      LOG(DEBUG) << "value = " << value << "\n";

      bufferOffset = ctx.length; // 更新

      LOG(DEBUG) << "bufferOffset = " << bufferOffset << "\n";
      return *this;
   }

   /*
         void getBlock(void* dest, size_t length)
         {
            if(likely(this->bufferOffset + length >= this->bufferOffset
                  && this->bufferOffset + length <= this->bufferSize) )
               std::memcpy(dest, this->buffer + this->bufferOffset, length);
            else
               setBad();

            this->bufferOffset += length;
         }

         template<typename Value>
         typename boost::enable_if_c<
               IsSerdesPrimitive<Value>::value,
               Deserializer&>::type
            operator%(Value& value)
         {
            getIntegral(value);
            return *this;
         }

         template<typename Value>
         typename boost::enable_if_c<
               !IsSerdesPrimitive<Value>::value
                  && has_deserializer<Value, &Value::serialize>::value,
               Deserializer&>::type
            operator%(Value& value)
         {
            Value::serialize(&value, *this);
            return *this;
         }

         template<typename Value, typename = typename SerializeAs<Value>::type>
         Deserializer& operator%(Value& value);

         void skip(size_t length)
         {
            if(unlikely(this->bufferOffset + length < this->bufferOffset
                  || this->bufferOffset + length > this->bufferSize) )
               setBad();

            this->bufferOffset += length;
         }
   */
public:
   /*
         template<typename T>
         Deserializer& operator%(std::list<T>& value)
         {
            getCollection(value);
            return *this;
         }

         template<typename T>
         Deserializer& operator%(std::vector<T>& value)
         {
            getCollection(value);
            return *this;
         }

         template<typename T, typename Compare>
         Deserializer& operator%(std::set<T, Compare>& value)
         {
            getCollection(value);
            return *this;
         }

         template<typename KeyT, typename ValueT, typename CompT, typename AllocT>
         Deserializer& operator%(std::map<KeyT, ValueT, CompT, AllocT>& map)
         {
            getMap(map);
            return *this;
         }

         template<typename T1, typename T2>
         Deserializer& operator%(std::pair<T1, T2>& pair)
         {
            return *this
               % pair.first
               % pair.second;
         }

         template<typename... ElemsT>
         Deserializer& operator%(std::tuple<ElemsT...>& tuple)
         {
            getTuple(tuple, std::integral_constant<size_t, sizeof...(ElemsT)>());
            return *this;
         }
   */
private:
   /*
         void getIntegral(bool& value) { value = getRaw<uint8_t>(); }
         void getIntegral(char& value) { value = getRaw<char>(); }
         void getIntegral(uint8_t& value) { value = getRaw<uint8_t>(); }
         void getIntegral(uint16_t& value) { value = LE_TO_HOST_16(getRaw<uint16_t>() ); }
         void getIntegral(uint32_t& value) { value = LE_TO_HOST_32(getRaw<uint32_t>() ); }
         void getIntegral(uint64_t& value) { value = LE_TO_HOST_64(getRaw<uint64_t>() ); }

         void getIntegral(int16_t& value)
         {
            value = bitcast<int16_t>(LE_TO_HOST_16(getRaw<uint16_t>() ) );
         }

         void getIntegral(int32_t& value)
         {
            value = bitcast<int32_t>(LE_TO_HOST_32(getRaw<uint32_t>() ) );
         }

         void getIntegral(int64_t& value)
         {
            value = bitcast<int64_t>(LE_TO_HOST_64(getRaw<uint64_t>() ) );
         }

         template<typename Raw>
         Raw getRaw()
         {
            Raw value = 0;
            getBlock(&value, sizeof(value) );
            return value;
         }

         template<typename Out, typename In>
         static Out bitcast(In in)
         {
            BOOST_STATIC_ASSERT(sizeof(In) == sizeof(Out) );
            Out out;
            memcpy(&out, &in, sizeof(in) );
            return out;
         }
   */
private:
   /*
         template<typename Collection>
         void getCollection(Collection& value)
         {
            unsigned offsetAtStart = this->bufferOffset;

            uint32_t totalLen;
            uint32_t size;

            if(ListSerializationHasLength<typename Collection::value_type>::value)
               *this % totalLen;

            *this % size;

            if(unlikely(!good() ) )
               return;

            value.clear();

            while(size > 0)
            {
               typename Collection::value_type item;

               *this % item;
               if (unlikely(!good()))
                  return;
               value.insert(value.end(), item);

               size -= 1;
            }

            if(unlikely(!good() ) )
               return;

            if(!ListSerializationHasLength<typename Collection::value_type>::value)
               return;

            if(unlikely(totalLen != this->bufferOffset - offsetAtStart) )
               setBad();
         }

         template<typename KeyT, typename ValueT, typename CompT, typename AllocT>
         void getMap(std::map<KeyT, ValueT, CompT, AllocT>& value)
         {
            unsigned offsetAtStart = this->bufferOffset;

            uint32_t totalLen;
            uint32_t size;

            if (MapSerializationHasLength<KeyT, ValueT>::value)
               *this % totalLen;

            *this % size;

            if (unlikely(!good()))
               return;

            value.clear();

            while (size > 0)
            {
               KeyT key;

               *this % key;
               if (unlikely(!good()))
                  return;
               *this % value[key];
               if (unlikely(!good()))
                  return;

               size -= 1;
            }

            if (unlikely(!good()))
               return;

            if (!MapSerializationHasLength<KeyT, ValueT>::value)
               return;

            if (unlikely(totalLen != this->bufferOffset - offsetAtStart))
               setBad();
         }

         template<size_t Idx, typename... ElemsT>
         void getTuple(std::tuple<ElemsT...>& tuple, std::integral_constant<size_t, Idx>)
         {
            *this % std::get<sizeof...(ElemsT) - Idx>(tuple);
            getTuple(tuple, std::integral_constant<size_t, Idx - 1>());
         }

         template<typename... ElemsT>
         void getTuple(std::tuple<ElemsT...>& tuple, std::integral_constant<size_t, 0>)
         {
         }
   */
};

inline void swap(Deserializer &a, Deserializer &b)
{
   a.swap(b);
}

class FindOwnerRespMsg;
class FindOwnerMsg;

template <typename T>
struct IsSerdesPrimitive
{
   typedef typename std::decay<T>::type value_type;
   enum
   {
      value =
          std::is_same<value_type, bool>::value || std::is_same<value_type, char>::value || std::is_same<value_type, unsigned char>::value // uint8_t is unsigned char in C++
          || std::is_same<value_type, short>::value                                                                                        // int16_t is short in C++
          || std::is_same<value_type, unsigned short>::value                                                                               // uint16_t is unsigned short in C++
          || std::is_same<value_type, int>::value                                                                                          // int32_t is int in C++
          || std::is_same<value_type, unsigned int>::value                                                                                 // uint32_t is unsigned int in C++
          || std::is_same<value_type, long long>::value                                                                                    // int64_t is long long in C++
          || std::is_same<value_type, unsigned long long>::value                                                                           // uint64_t is unsigned long long in C++
   };
};

static void deserializeHeader(char *buf, size_t bufLen, NetMessageHeader *outHeader);

template <typename T>
struct ListSerializationHasLength : std::true_type
{
};

template <>
struct ListSerializationHasLength<char> : std::false_type
{
};

#include <inttypes.h>

// byte order transformation macros for 16-, 32-, 64-bit types

inline uint16_t byteswap16(uint16_t u)
{
   uint16_t high = u >> 8;
   uint16_t low = u & 0xFF;

   return high | (low << 8);
}

inline uint32_t byteswap32(uint32_t u)
{
   uint32_t high = u >> 16;
   uint32_t low = u & 0xFFFF;

   return byteswap16(high) | (uint32_t(byteswap16(low)) << 16);
}

inline uint64_t byteswap64(uint64_t u)
{
   uint64_t high = u >> 32;
   uint64_t low = u & 0xFFFFFFFF;

   return byteswap32(high) | (uint64_t(byteswap32(low)) << 32);
}

#if BYTE_ORDER == BIG_ENDIAN // BIG_ENDIAN

#define HOST_TO_LE_16(value) (::byteswap16(value))
#define HOST_TO_LE_32(value) (::byteswap32(value))
#define HOST_TO_LE_64(value) (::byteswap64(value)) // 这是一个宏定义，用于将64位整数从主机字节序转换为小端字节

#else // LITTLE_ENDIAN

#define HOST_TO_LE_16(value) (value)
#define HOST_TO_LE_32(value) (value)
#define HOST_TO_LE_64(value) (value)

#endif // BYTE_ORDER

template <typename T>
struct SerializeAs
{
};

class Serializer
{
public:
   Serializer()
       : buffer(nullptr), bufferSize(-1), bufferOffset(0)
   {
   }

   Serializer(char *buffer, unsigned bufferSize)
       : buffer(buffer), bufferSize(bufferSize), bufferOffset(0)
   {
   }

   Serializer(Serializer &&other)
       : buffer(nullptr), bufferSize(-1), bufferOffset(0)
   {
      swap(other);
   }

   Serializer &operator=(Serializer &&other)
   {
      Serializer(std::move(other)).swap(*this);
      return *this;
   }

private:
   char *buffer;
   unsigned bufferSize;
   unsigned bufferOffset;

   template <typename Value, void (*Fn)(const Value *, Serializer &)>
   struct has_serializer : std::true_type
   {
   };

   Serializer(char *buffer, unsigned bufferSize, unsigned bufferOffset)
       : buffer(buffer), bufferSize(bufferSize), bufferOffset(bufferOffset)
   {
   }

public:
   bool isReading() const { return false; }
   bool isWriting() const { return true; }

   unsigned size() const
   {
      return this->bufferOffset;
   }

   bool good() const
   {
      return this->bufferSize && this->bufferOffset <= this->bufferSize;
   }

   void swap(Serializer &other)
   {
      std::swap(buffer, other.buffer);
      std::swap(bufferSize, other.bufferSize);
      std::swap(bufferOffset, other.bufferOffset);
   }

   Serializer mark() const
   {
      return {buffer, bufferSize, bufferOffset};
   }

   void putBlock(const void *source, size_t length)
   {
      if (this->buffer && this->bufferOffset + length >= this->bufferOffset && this->bufferOffset + length <= this->bufferSize)
         std::memcpy(this->buffer + this->bufferOffset, source, length);
      else
         this->bufferSize = 0;

      this->bufferOffset += length;
   }

   void skip(unsigned size)
   {
      while (size > 0)
      {
         putBlock("\0\0\0\0\0\0\0\0", std::min<unsigned>(size, 8));
         size -= std::min<unsigned>(size, 8);
      }
   }

   // 重载运算符%以序列化NetMessageHeader类型的数据
   Serializer &operator%(const NetMessageHeader &msgHeader)
   {
      /*
           // 确保有足够的空间来序列化NetMessageHeader
           if (bufferOffset + sizeof(NetMessageHeader) <= bufferSize) {
               // 将NetMessageHeader的成员复制到缓冲区
               std::memcpy(buffer + bufferOffset, &msgHeader, sizeof(NetMessageHeader));
               // 更新位置指针
               bufferOffset += sizeof(NetMessageHeader);
           } else {
               // 处理错误，比如抛出异常或设置错误标志
               LOG(DEBUG)<<"Serializer out of range \n";
               throw std::runtime_error("Serializer out of range");
           }
      */
      SerializeCtx ctx = {
          .data = buffer,
          .length = bufferOffset};
      // 序列化信息头部
      LOG(DEBUG) << "-- 序列化NetMessageHeader ---\n";
      // LOG(DEBUG)<<"1ctx.length = "<<ctx.length<<"\n";
      Serialization_serializeUInt(&ctx, msgHeader.msgLength); // 4
      // LOG(DEBUG)<<"2ctx.length = "<<ctx.length<<"\n";
      Serialization_serializeUShort(&ctx, msgHeader.msgFeatureFlags); // 2
      // LOG(DEBUG)<<"3ctx.length = "<<ctx.length<<"\n";

      Serialization_serializeChar(&ctx, msgHeader.msgCompatFeatureFlags); // 1
      // LOG(DEBUG)<<"4ctx.length = "<<ctx.length<<"\n";

      Serialization_serializeChar(&ctx, msgHeader.msgFlags); // 1
      // LOG(DEBUG)<<"5ctx.length = "<<ctx.length<<"\n";

      Serialization_serializeUInt64(&ctx, NETMSG_PREFIX); // 8
      // LOG(DEBUG)<<"6ctx.length = "<<ctx.length<<"\n";

      Serialization_serializeUShort(&ctx, msgHeader.msgType); // 2
      // LOG(DEBUG)<<"7ctx.length = "<<ctx.length<<"\n";

      Serialization_serializeUShort(&ctx, msgHeader.msgTargetID); // 2
      // LOG(DEBUG)<<"8ctx.length = "<<ctx.length<<"\n";

      Serialization_serializeUInt(&ctx, msgHeader.msgUserID); // 4
      // LOG(DEBUG)<<"9ctx.length = "<<ctx.length<<"\n";

      Serialization_serializeUInt64(&ctx, msgHeader.msgSequence); // 8
      // LOG(DEBUG)<<"10ctx.length = "<<ctx.length<<"\n";

      Serialization_serializeUInt64(&ctx, msgHeader.msgSequenceDone); // 8
      // LOG(DEBUG)<<"end --- ctx.length = "<<ctx.length<<"\n";

      // printNetMessageHeader(msgHeader);
      bufferOffset = ctx.length; // 更新
      // LOG(DEBUG)<<"bufferOffset = "<<bufferOffset<<"\n";
      //       for(int i=0;i<bufferOffset;i++)
      //       {
      //          LOG(DEBUG)<<buffer[i];
      //       }
      //    LOG(DEBUG)<<"\n";
      NetMessageHeader header;
      deserializeHeader(buffer, bufferSize, &header);
      LOG(DEBUG) << "(header->msgType = " << header.msgType << "\n";
      // printNetMessageHeader(header);s

      return *this;
   }

   //  template<typename Serializer>
   //   friend Serializer& operator%(Serializer& ser, const FindOwnerRespMsg& msg) {
   //       msg.serialize(ser); // 调用 msg 的 serialize 方法
   //       return ser;
   //    }

   // 重载运算符%以序列化uint32_t类型的数据
   Serializer &operator%(uint32_t value)
   {
      //      if (bufferOffset + sizeof(uint32_t) <= bufferSize) {
      //          // 将uint32_t值的字节拷贝到缓冲区
      //          std::memcpy(buffer + bufferOffset, &value, sizeof(uint32_t));
      //          // 更新位置指针
      //          bufferOffset += sizeof(uint32_t);
      //      } else {
      //          // 处理错误，比如抛出异常或设置错误标志
      //          throw std::runtime_error("Serializer out of range");
      //      }
      //
      SerializeCtx ctx = {
          .data = buffer,
          .length = bufferOffset};
      Serialization_serializeUInt(&ctx, value);
      bufferOffset = ctx.length; // 更新
      LOG(DEBUG) << "bufferOffset = " << bufferOffset << "\n";

      return *this;
   }

   // 重载运算符%以序列化uint32_t类型的数据
   Serializer &operator%(int value)
   {
      //      if (bufferOffset + sizeof(uint32_t) <= bufferSize) {
      //          // 将uint32_t值的字节拷贝到缓冲区
      //          std::memcpy(buffer + bufferOffset, &value, sizeof(uint32_t));
      //          // 更新位置指针
      //          bufferOffset += sizeof(uint32_t);
      //      } else {
      //          // 处理错误，比如抛出异常或设置错误标志
      //          throw std::runtime_error("Serializer out of range");
      //      }
      //
      SerializeCtx ctx = {
          .data = buffer,
          .length = bufferOffset};
      Serialization_serializeInt(&ctx, value);
      bufferOffset = ctx.length; // 更新
      LOG(DEBUG) << "bufferOffset = " << bufferOffset << "\n";

      return *this;
   }

   // 重载运算符%以序列化uint32_t类型的数据
   Serializer &operator%(const long unsigned int value)
   {
      //      if (bufferOffset + sizeof(uint32_t) <= bufferSize) {
      //          // 将uint32_t值的字节拷贝到缓冲区
      //          std::memcpy(buffer + bufferOffset, &value, sizeof(uint32_t));
      //          // 更新位置指针
      //          bufferOffset += sizeof(uint32_t);
      //      } else {
      //          // 处理错误，比如抛出异常或设置错误标志
      //          throw std::runtime_error("Serializer out of range");
      //      }
      //
      SerializeCtx ctx = {
          .data = buffer,
          .length = bufferOffset};
      Serialization_serializeUInt64(&ctx, value);
      bufferOffset = ctx.length; // 更新
      LOG(DEBUG) << "bufferOffset = " << bufferOffset << "\n";

      return *this;
   }

   // Serializer& operator%(const FindOwnerMsg& msg) {
   //       //   FindOwnerRespMsg::serialize(const_cast<FindOwnerRespMsg*>(&msg), *this);
   //       SerializeCtx ctx = {
   //                .data = buffer,
   //                .length=bufferOffset
   //             };
   //             Serialization_serializeInt(&ctx,mag.get ());

   //              bufferOffset =ctx.length;//更新
   //       LOG(DEBUG)<<"bufferOffset = "<<bufferOffset<<"\n";
   //         return *this;
   //     }

   // 重载 operator% 来序列化 FindOwnerRespMsg 类型
   //  Serializer& operator%(const FindOwnerRespMsg& msg) {
   //    //   FindOwnerRespMsg::serialize(const_cast<FindOwnerRespMsg*>(&msg), *this);
   //    SerializeCtx ctx = {
   //             .data = buffer,
   //             .length=bufferOffset
   //          };
   //          Serialization_serializeInt(&ctx,msg.getResult());

   //           bufferOffset =ctx.length;//更新
   //    LOG(DEBUG)<<"bufferOffset = "<<bufferOffset<<"\n";
   //      return *this;
   //  }

   // template<typename Value>
   // typename std::enable_if<IsSerdesPrimitive<Value>::value, Serializer&>::type
   // operator%(Value value)
   // {
   //    putIntegral(value);
   //    return *this;
   // }

   // template<typename Value>
   // typename std::enable_if<
   //       !IsSerdesPrimitive<Value>::value &&
   //       std::is_member_function_pointer<decltype(&Value::serialize)>::value,
   //       Serializer&>::type
   // operator%(const Value& value)
   // {
   //    Value::serialize(&value, *this);
   //    return *this;
   // }

   // template<typename Value, typename = typename SerializeAs<Value>::type>
   // Serializer& operator%(const Value& value);

public:
   template <typename T>
   Serializer &operator%(const std::vector<T> &value)
   {

      LOG(DEBUG) << "go into Serializer& operator%(const std::vector<T>& value) \n";
      LOG(DEBUG) << "value.size = " << value.size() << "\n";
      putCollection<ListSerializationHasLength<T>::value>(value.begin(), value.end());
      LOG(DEBUG) << "go out of  Serializer& operator%(const std::vector<T>& value) \n";

      return *this;
   }
   char *getbuffer()
   {
      return this->buffer;
   }
   unsigned getbufferoffset()
   {
      return this->bufferOffset;
   }
   unsigned getbuffersize()
   {
      return this->bufferSize;
   }
   bool setbufferoffset(unsigned value)
   {
      if (value <= this->bufferSize)
      {
         this->bufferOffset = value;

         return true;
      }
      else
      {
         LOG(DEBUG) << "[Class Serializer] Failed to set bufferoffset \n";
         return false;
      }
   }

private:
   void putIntegral(bool value) { putRaw<uint8_t>(value ? 1 : 0); }
   void putIntegral(char value) { putRaw(value); }
   void putIntegral(uint8_t value) { putRaw(value); }
   void putIntegral(int16_t value) { putRaw(static_cast<uint16_t>(HOST_TO_LE_16(value))); }
   void putIntegral(uint16_t value) { putRaw(static_cast<uint16_t>(HOST_TO_LE_16(value))); }
   void putIntegral(int32_t value) { putRaw(static_cast<uint32_t>(HOST_TO_LE_32(value))); }
   void putIntegral(uint32_t value) { putRaw(static_cast<uint32_t>(HOST_TO_LE_32(value))); }
   void putIntegral(int64_t value) { putRaw(static_cast<uint64_t>(HOST_TO_LE_64(value))); }
   void putIntegral(uint64_t value) { putRaw(static_cast<uint64_t>(HOST_TO_LE_64(value))); }

   template <typename Raw>
   void putRaw(Raw value)
   {
      putBlock(&value, sizeof(value));
   }

   template <bool HasLengthField, typename Iter>
   void putCollection(const Iter begin, const Iter end)
   {
      Serializer atStart = mark();
      unsigned offsetAtStart = this->bufferOffset;
      uint32_t elements = 0; // list<T> is slow for size()
      LOG(DEBUG) << "HasLengthField = " << HasLengthField << "\n";
      if (HasLengthField)
         *this % uint32_t(0);

      *this % uint32_t(0); // number of elements

      for (Iter it = begin; it != end; ++it)
      {
         *this % *it;
         elements += 1;
         LOG(DEBUG) << "@@ \n";
      }

      if (HasLengthField)
         atStart % uint32_t(this->bufferOffset - offsetAtStart);

      atStart % elements;
   }
};

static void fixLengthField(Serializer &ser, uint32_t actualLength)
{
   ser % actualLength;
}
class NetMessage_class
{
   // friend class AbstractNetMessageFactory;
   // friend class TestMsgSerializationBase;

public:
   virtual ~NetMessage_class() {}

   static void deserializeHeader(char *buf, size_t bufLen, NetMessageHeader *outHeader)
   {
      Deserializer des(buf, bufLen);
      des % *outHeader;
      if (!des.good())
      {
         LOG(DEBUG) << "!des.good() \n";
         outHeader->msgType = NETMSGTYPE_Invalid;
      }
   }

   virtual bool supportsMirroring() const { return false; }

   virtual void serializePayload(Serializer &ser) const = 0; //@@
   // void serializePayload(Serializer& ser)
   // {
   //    // Serialization_serializeUInt(&ctx,msgHeader.msgLength);
   // }

   virtual bool deserializePayload(const char *buf, size_t bufLen) = 0;

protected:
   NetMessage_class(unsigned short msgType)
   {
      this->msgHeader.msgLength = 0;
      this->msgHeader.msgFeatureFlags = 0;
      this->msgHeader.msgCompatFeatureFlags = 0;
      this->msgHeader.msgFlags = 0;
      this->msgHeader.msgType = msgType;
      LOG(DEBUG) << "@@msgtype = " << msgType << "\n";
      this->msgHeader.msgUserID = NETMSG_DEFAULT_USERID;
      this->msgHeader.msgTargetID = 0;
      this->msgHeader.msgSequence = 0;
      this->msgHeader.msgSequenceDone = 0;

      this->releaseSockAfterProcessing = true;
   }

   // virtual void serializePayload(Serializer& ser) const = 0;//@@
   // // void serializePayload(Serializer& ser)
   // // {
   // //    // Serialization_serializeUInt(&ctx,msgHeader.msgLength);
   // // }

   // virtual bool deserializePayload(const char* buf, size_t bufLen) = 0;

private:
   NetMessageHeader msgHeader;

   bool releaseSockAfterProcessing; /* false if sock was already released during
                                       processIncoming(), e.g. due to early response. */

   std::vector<char> backingBuffer;

public:
   NetMessageHeader getmsgHeader()
   {
      return this->msgHeader;
   }
   std::vector<char> getbackingBuffer()
   {
      return this->backingBuffer;
   }

   unsigned short getmsgType() //@@ note:名字修改了
   {
      return this->msgHeader.msgType;
   }
   std::pair<bool, unsigned> serializeMessage(char *buf, size_t bufLen) const
   {
      // LOG(DEBUG)<<"-----------1-----------\n";

      Serializer ser(buf, bufLen);
      Serializer atStart = ser.mark();
      // LOG(DEBUG)<<"-----------2-----------\n";

      ser % msgHeader;
      LOG(DEBUG) << "-----------3-----------\n";

      serializePayload(ser); // 问题的关键出在这里 ，
      LOG(DEBUG) << "getbufferoffset = " << ser.getbufferoffset() << "\t, getbuffersize = " << ser.getbuffersize() << "\n";
      // LOG(DEBUG)<<"-----------4-----------\n";

      // fix message length in header and serialize header again to fix the message length
      // NetMessageHeader::fixLengthField(atStart, ser.size() );
      LOG(DEBUG) << "ser.size() = " << ser.size() << "\n"; // 102
      fixLengthField(atStart, ser.size());                 // @@
      LOG(DEBUG) << "atStart.getbufferoffset() =" << atStart.getbufferoffset() << "\n";
      LOG(DEBUG) << "-----------5-----------\n";

      bool res = ser.good();
      LOG(DEBUG) << "ser.good() = " << ser.good() << "\n";
      LOG(DEBUG) << "getbufferoffset = " << ser.getbufferoffset() << "\t, getbuffersize = " << ser.getbuffersize() << "\n";

      return std::make_pair(ser.good(), ser.size());
   }

   // Note: You probably rather want to call addMsgHeaderFeatureFlag() instead of this.

   void setMsgHeaderFeatureFlags(unsigned msgFeatureFlags)
   {
      this->msgHeader.msgFeatureFlags = msgFeatureFlags;
   }

   /**
    * Returns all feature flags that are supported by this message. Defaults to "none", so this
    * method needs to be overridden by derived messages that actually support header feature
    * flags.
    *
    * @return combination of all supported feature flags
    */
   virtual unsigned getSupportedHeaderFeatureFlagsMask() const
   {
      return 0;
   }
   // Check if the msg sender has set an incompatible feature flag.
   // @return false if an incompatible feature flag was set

   bool checkHeaderFeatureFlagsCompat() const
   {
      unsigned unsupportedFlags = ~getSupportedHeaderFeatureFlagsMask();
      if (msgHeader.msgFeatureFlags & unsupportedFlags)
         return false; // an unsupported flag was set

      return true;
   }

   /*


      // getters & setters

      unsigned short getMsgType() const
      {
         return msgHeader.msgType;
      }

      //Note: calling this method is expensive, so use it only in error/debug code paths.
       // @return human-readable message type (intended for log messages).

      std::string getMsgTypeStr() const
      {
         return netMessageTypeToStr(msgHeader.msgType);
      }

      unsigned getMsgHeaderFeatureFlags() const
      {
         return msgHeader.msgFeatureFlags;
      }


      // Test flag. (For convenience and readability.)
       // @return true if given flag is set.

      bool isMsgHeaderFeatureFlagSet(unsigned flag) const
      {
         return (this->msgHeader.msgFeatureFlags & flag) != 0;
      }

      // Add another flag without clearing the previously set flags.
       // Note: The receiver will reject this message if it doesn't know the given feature flag.

      void addMsgHeaderFeatureFlag(unsigned flag)
      {
         this->msgHeader.msgFeatureFlags |= flag;
      }

      // Remove a certain flag without clearing the other previously set flags.

      void unsetMsgHeaderFeatureFlag(unsigned flag)
      {
         this->msgHeader.msgFeatureFlags &= ~flag;
      }

      uint8_t getMsgHeaderCompatFeatureFlags() const
      {
         return msgHeader.msgCompatFeatureFlags;
      }

      void setMsgHeaderCompatFeatureFlags(uint8_t msgCompatFeatureFlags)
      {
         this->msgHeader.msgCompatFeatureFlags = msgCompatFeatureFlags;
      }

      //Test flag. (For convenience and readability.)
       //@return true if given flag is set.

      bool isMsgHeaderCompatFeatureFlagSet(uint8_t flag) const
      {
         return (this->msgHeader.msgCompatFeatureFlags & flag) != 0;
      }

      //Add another flag without clearing the previously set flags.
       // Note: "compat" means these flags might not be understood and will then just be ignored by
       // the receiver (e.g. if the receiver is an older fhgfs version).

      void addMsgHeaderCompatFeatureFlag(uint8_t flag)
      {
         this->msgHeader.msgCompatFeatureFlags |= flag;
      }

      unsigned getMsgHeaderUserID() const
      {
         return msgHeader.msgUserID;
      }

      void setMsgHeaderUserID(unsigned userID)
      {
         this->msgHeader.msgUserID = userID;
      }

      unsigned getMsgHeaderTargetID() const
      {
         return msgHeader.msgTargetID;
      }

      // @param targetID this has to be an actual targetID (not a groupID); (default is 0 if
      // targetID is not applicable).

      void setMsgHeaderTargetID(uint16_t targetID)
      {
         this->msgHeader.msgTargetID = targetID;
      }

      bool getReleaseSockAfterProcessing() const
      {
         return releaseSockAfterProcessing;
      }

      void setReleaseSockAfterProcessing(bool releaseSockAfterProcessing)
      {
         this->releaseSockAfterProcessing = releaseSockAfterProcessing;
      }

      uint64_t getSequenceNumber() const { return msgHeader.msgSequence; }
      void     setSequenceNumber(uint64_t value) { msgHeader.msgSequence = value; }

      uint64_t getSequenceNumberDone() const { return msgHeader.msgSequenceDone; }
      void     setSequenceNumberDone(uint64_t value) { msgHeader.msgSequenceDone = value; }

      uint32_t getLength() const { return msgHeader.msgLength; }

      uint8_t getFlags() const { return msgHeader.msgFlags; }
      bool hasFlag(uint8_t flag) const { return msgHeader.msgFlags & flag; }
      void addFlag(uint8_t flag) { msgHeader.msgFlags |= flag; }
      void removeFlag(uint8_t flag) { msgHeader.msgFlags &= ~flag; }
   */
};

template <typename Derived> // 弃用
class NetMessageSerdes : public NetMessage_class
{
protected:
   typedef NetMessageSerdes<Derived> BaseType;

   NetMessageSerdes(unsigned short msgType)
       : NetMessage_class(msgType)
   {
   }

   void serializePayload(Serializer &ser) const //@@
   {
      LOG(DEBUG) << "-----------serializePayload-----------\n";

      // ser % *(Derived*) this;
      LOG(DEBUG) << "-----------serializePayload----------\n";
   }

   bool deserializePayload(const char *buf, size_t bufLen)
   {
      Deserializer des(buf, bufLen);
      // des % *(Derived*) this;
      return des.good();
   }
};
// #include <boost/type_traits/remove_cv.hpp>//提示fatal error: boost/type_traits/remove_cv.hpp: No such file or directory

// template<typename Object, typename Back = typename boost::remove_cv<Object>::type>//@@ 我不用boost库
// template<typename Object, typename Back = std::remove_cv<Object>::type>
template <typename Object, typename Back = typename std::remove_cv<Object>::type> // 改成这个
struct BackedPtrDes
{
   Object **ptr;
   Back *backing;

   BackedPtrDes(Object *&ptr, Back &backing)
       : ptr(&ptr), backing(&backing)
   {
   }

   static Object *get(Object &backing)
   {
      return &backing;
   }

   template <typename T>
   static Object *get(std::unique_ptr<T> &backing)
   {
      return backing.get();
   }

   friend Deserializer &operator%(Deserializer &des, BackedPtrDes value)
   {
      des % *value.backing;
      *value.ptr = BackedPtrDes::get(*value.backing);
      return des;
   }
};

template <typename Object>
struct BackedPtrSer
{
   const Object *source;

   BackedPtrSer(const Object &source)
       : source(&source)
   {
   }

   friend Serializer &operator%(Serializer &ser, const BackedPtrSer &value)
   {
      return ser % *value.source;
   }
};

namespace serdes
{

   template <typename Source, typename Backing>
   inline BackedPtrSer<Source> backedPtr(Source *const &source, const Backing &backing)
   {
      return BackedPtrSer<Source>(*source);
   }

   template <typename Source, typename Backing>
   inline BackedPtrSer<Source> backedPtr(const Source *const &source, const Backing &backing)
   {
      return BackedPtrSer<Source>(*source);
   }

   // template<typename Object>
   // inline BackedPtrDes<Object> backedPtr(Object*& ptr, Object& backing)
   // {
   //    return BackedPtrDes<Object>(ptr, backing);
   // }

   // template<typename Object>
   // inline BackedPtrDes<const Object> backedPtr(const Object*& ptr, Object& backing)
   // {
   //    return BackedPtrDes<const Object>(ptr, backing);
   // }

   // template<typename Object>
   // inline BackedPtrDes<Object, std::unique_ptr<Object>>
   //    backedPtr(Object*& ptr, std::unique_ptr<Object>& backing)
   // {
   //    return BackedPtrDes<Object, std::unique_ptr<Object>>(ptr, backing);
   // }
}
// class FindOwnerMsg : public NetMessageSerdes<FindOwnerMsg>
// class FindOwnerMsg : public NetMessageSerdes<FindOwnerMsg>
class FindOwnerMsg : public NetMessage_class
{
   friend class AbstractNetMessageFactory;

public:
   /**
    * @param path just a reference, so do not free it as long as you use this object!
    * @param entryID just a reference, so do not free it as long as you use this object!
    */
   FindOwnerMsg(Path *path, unsigned searchDepth, EntryInfo_class *entryInfo, int currentDepth)
       // : BaseType(NETMSGTYPE_FindOwner) //@@ 我注释的
       : NetMessage_class(NETMSGTYPE_FindOwner)
   {
      this->path = path;
      this->searchDepth = searchDepth;

      this->entryInfoPtr = entryInfo;

      this->currentDepth = currentDepth;
   }

   // FindOwnerMsg() : BaseType(NETMSGTYPE_FindOwner)//@@ 我注释的
   FindOwnerMsg() : NetMessage_class(NETMSGTYPE_FindOwner) //@@ 我注释的

   {
   }

   // template<typename This, typename Ctx>
   // static void serialize(This obj, Ctx& ctx) //这个函数八成会用上 //@@ 我注释的
   // {
   //    LOG(DEBUG)<<"FindOwnerMsg::serialize() \n";
   //    ctx
   //       % obj->searchDepth
   //       % obj->currentDepth;
   //       // % serdes::backedPtr(obj->entryInfoPtr, obj->entryInfo) //@@ 这个对应函数也得改
   //       // % serdes::backedPtr(obj->path, obj->parsed.path);//
   //          // % serdes::backedPtr(obj->path, const_cast<Path&>(&obj->parsed.path)); // 使用新的重载版本
   //    LOG(DEBUG)<<"FindOwnerMsg::serialize() \n";

   // }
   // FindOwnerMsg 类中
   virtual void serializePayload(Serializer &ser) const override
   {
      // 提供 serializePayload 函数的实现
      LOG(DEBUG) << "FindOwnerMsg::serialize()--- begin \n";
      SerializeCtx ctx = {
          .data = ser.getbuffer(),
          .length = ser.getbufferoffset()};

      LOG(DEBUG) << "1ctx.length = " << ctx.length << "\n";

      Serialization_serializeUInt(&ctx, this->searchDepth); // 4
      LOG(DEBUG) << "searchDepth = " << this->searchDepth << "\n";
      LOG(DEBUG) << "2ctx.length = " << ctx.length << "\n";

      Serialization_serializeUInt(&ctx, this->currentDepth); // 4
      LOG(DEBUG) << "currentDepth = " << this->currentDepth << "\n";
      LOG(DEBUG) << "3ctx.length = " << ctx.length << "\n";

      // 先entryInfoPtr ,Note:不知道顺序应该如何设置 ，现在知道了，参考 class entryinfo 中的serialize 函数
      Serialization_serializeUInt(&ctx, this->entryInfoPtr->getEntryType()); // 4
      LOG(DEBUG) << "getEntryType = " << this->entryInfoPtr->getEntryType() << "\n";
      LOG(DEBUG) << "4ctx.length = " << ctx.length << "\n";

      Serialization_serializeInt(&ctx, this->entryInfoPtr->getFeatureFlags()); // 4
      LOG(DEBUG) << "feature flag  = " << this->entryInfoPtr->getFeatureFlags() << "\n";
      LOG(DEBUG) << "5ctx.length = " << ctx.length << "\n";

      LOG(DEBUG) << "parententryID = " << this->entryInfoPtr->getParentEntryID() << "\t , " << this->entryInfoPtr->getParentEntryID().length() << "\n";

      //  char* parentEntryIDPtr = const_cast<char*>(this->entryInfoPtr->getParentEntryID().c_str());

      size_t parentEntryID_len = this->entryInfoPtr->getParentEntryID().size() + 1; // +1 for the null terminator
      LOG(DEBUG) << "parentEntryID_len = " << parentEntryID_len << "\n";
      LOG(DEBUG) << "this->entryInfoPtr->getParentEntryID() = " << this->entryInfoPtr->getParentEntryID() << "\n";
      char *parentEntryID_Ptr = new char[parentEntryID_len];
      // this->entryInfoPtr->getParentEntryID().copy(parentEntryID_Ptr, parentEntryID_len);//@@ xyp_insert
      strcpy(parentEntryID_Ptr, this->entryInfoPtr->getParentEntryID().c_str());
      LOG(DEBUG) << " parententryID char = " << parentEntryID_Ptr << "\t , " << strlen(parentEntryID_Ptr) << "\n";

      Serialization_serializeStrAlign4(&ctx, strlen(parentEntryID_Ptr), parentEntryID_Ptr); // 8
      LOG(DEBUG) << "6ctx.length = " << ctx.length << "\n";

      // char* EntryIDPtr = const_cast<char*>(this->entryInfoPtr->getEntryID().c_str());
      size_t EntryID_len = this->entryInfoPtr->getEntryID().size() + 1; // +1 for the null terminator
      char *EntryID_Ptr = new char[EntryID_len];
      // this->entryInfoPtr->getEntryID().copy(EntryID_Ptr, EntryID_len);
      strcpy(EntryID_Ptr, this->entryInfoPtr->getEntryID().c_str());

      LOG(DEBUG) << " EntryID char = " << EntryID_Ptr << "\t , " << strlen(EntryID_Ptr) << "\n";

      LOG(DEBUG) << "^^EntryIDPtr = " << EntryID_Ptr << "\n";
      Serialization_serializeStrAlign4(&ctx, strlen(EntryID_Ptr), EntryID_Ptr); // 12
      LOG(DEBUG) << "7ctx.length = " << ctx.length << "\n";

      // char* fileNamePtr = const_cast<char*>(this->entryInfoPtr->getFileName().c_str());

      size_t fileName_len = this->entryInfoPtr->getFileName().size() + 1; // +1 for the null terminator
      char *fileName_Ptr = new char[fileName_len];
      // this->entryInfoPtr->getFileName().copy(fileName_Ptr, fileName_len);////@@ xyp_insert
      strcpy(fileName_Ptr, this->entryInfoPtr->getFileName().c_str());

      LOG(DEBUG) << " fileName char = " << fileName_Ptr << "\t , " << strlen(fileName_Ptr) << "\n";

      Serialization_serializeStrAlign4(&ctx, strlen(fileName_Ptr), fileName_Ptr); // 12
      LOG(DEBUG) << "8ctx.length = " << ctx.length << "\n";

      Serialization_serializeUInt(&ctx, this->entryInfoPtr->getOwnerNodeID().value); // 4
      LOG(DEBUG) << "9ctx.length = " << ctx.length << "\n";

      // padding for 4-byte alignment
      Serialization_serializeUShort(&ctx, 0);
      LOG(DEBUG) << "10ctx.length = " << ctx.length << "\n"; // 102

      // 后 path
      //   char* pathCharPtr = const_cast<char*>(this->path->getpathStr().c_str());

      size_t len = this->path->getpathStr().size() + 1; // +1 for the null terminator
      char *pathCharPtr = new char[len];
      this->path->getpathStr().copy(pathCharPtr, len);

      //   char* pathCharPtr = const_cast<char*>(this->path->getpathStr().data());

      LOG(DEBUG) << "strlen(pathCharPtr) = " << strlen(pathCharPtr) << "\t,";
      LOG(DEBUG) << "^^^pathCharPtr = " << pathCharPtr << "\n";

      LOG(DEBUG) << "^^^this->path->getpathStr() = " << this->path->getpathStr() << "\t, len =" << this->path->getpathStr().length() << "\n";

      for (int i = 0; i < searchDepth; i++)
      {
         LOG(DEBUG) << "path[" << i << "]=" << (*(this->path))[i] << "\n";
      }

      Serialization_serializeStr(&ctx, strlen(pathCharPtr), pathCharPtr); // 10
      // 使用 cstr 之后，记得删除分配的内存
      delete[] pathCharPtr;
      LOG(DEBUG) << "11ctx.length = " << ctx.length << "\n"; // 102

      ser.setbufferoffset(ctx.length);
      //   ser % (this->path->getdirSeparators());
      //   ctx.length=ser.getbufferoffset();
      LOG(DEBUG) << "12ctx.length = " << ctx.length << "\n"; //

      //   bufferOffset =ctx.length;//更新
      bool res = ser.setbufferoffset(ctx.length);
      if (res == false)
      {
         LOG(DEBUG) << "[class findownermsg ] serializePayload error \n";
      }
      LOG(DEBUG) << "bufferOffset = " << ctx.length << "\t, ser.buffersize" << ser.getbuffersize() << "\n";
   }

   virtual bool deserializePayload(const char *buf, size_t bufLen) override
   {

      return true;
   }

private:
   uint32_t searchDepth;
   uint32_t currentDepth;

   // for serialization
   Path *path;                    // not owned by this object!
   EntryInfo_class *entryInfoPtr; // not owned by this object!

   // for deserialization
   struct
   {
      Path path;
   } parsed;
   EntryInfo_class entryInfo;

public:
   Path &getPath()
   {
      return *path;
   }

   unsigned getSearchDepth()
   {
      return searchDepth;
   }

   unsigned getCurrentDepth()
   {
      return currentDepth;
   }

   EntryInfo_class *getEntryInfo()
   {
      return &this->entryInfo;
   }
   EntryInfo_class *getEntryInfoPtr() //@@
   {
      return this->entryInfoPtr;
   }
};

// class FindOwnerRespMsg : public NetMessageSerdes<FindOwnerRespMsg>
// class FindOwnerRespMsg : public NetMessageSerdes<FindOwnerRespMsg>
class FindOwnerRespMsg : public NetMessage_class

// class FindOwnerRespMsg //@@ 我改的
{
public:
   /**
    * @param entryInfo just a reference, so do not free it as long as you use this object!
    */
   FindOwnerRespMsg(int result, EntryInfoWithDepth *entryInfo) :                                                                                     //  result(result), entryInfoPtr(entryInfo)
                                                                 NetMessage_class(NETMSGTYPE_FindOwnerResp), result(result), entryInfoPtr(entryInfo) //@@ 我改的
   {
   }

   FindOwnerRespMsg() : NetMessage_class(NETMSGTYPE_FindOwnerResp) //@@ 我改的
   {
   }

   // template<typename This, typename Ctx>//@@ 我改的
   // static void serialize(This obj, Ctx& ctx)
   // {
   //    ctx
   //       % obj->result;
   //       % serdes::backedPtr(obj->entryInfoPtr, obj->entryInfo);
   //       // 直接构造 BackedPtrDes 对象并传递给 % 运算符
   //       // % BackedPtrDes<Object, std::unique_ptr<Object>>(obj->entryInfoPtr, obj->entryInfo);
   // }
   virtual void serializePayload(Serializer &ser) const override
   {
      LOG(DEBUG) << "FindOwnerRespMsg serializePayload \n";
   }

   virtual bool deserializePayload(const char *buf, size_t bufLen) override
   { //$$也要重写
      LOG(DEBUG) << "FindOwnerRespMsg deserializePayload \n";
      DeserializeCtx ctx = {
          .data = buf,
          // .length=sizeof(value)
          .length = bufLen

      };
      LOG(DEBUG) << "deserializer \n";
      LOG(DEBUG) << "before ctx.length = " << ctx.length << "\n";
      Serialization_deserializeInt(&ctx, &this->result);
      LOG(DEBUG) << "result = " << result << "\n";
      if (result == 8)
      {
         LOG(DEBUG) << "路径不存在 \n";
      }
      // entryinfowithdepth ::
      //% serdes::base<EntryInfo_class>(obj)
      //% obj->entryDepth;
      // 先 EntryInfo_class

      // 1
      //  DirEntryType entryType;//=this->entryInfo.getEntryType();
      unsigned int entrytype;
      // Serialization_deserializeUInt(&ctx,(unsigned int *)entryType );
      Serialization_deserializeUInt(&ctx, &entrytype);
      DirEntryType entryType = static_cast<DirEntryType>(entrytype);

      LOG(DEBUG) << " entryType = " << entryType << "\t, entrytype = " << entrytype << "\n";
      // 2
      int featureFlags;
      Serialization_deserializeInt(&ctx, &featureFlags);
      LOG(DEBUG) << " featureFlags = " << featureFlags << "\t, ";

      // 3
      unsigned ParentEntryID_len;
      const char *ParentEntryID = nullptr;
      LOG(DEBUG) << "----------- 3 -------\n";
      Serialization_deserializeStrAlign4(&ctx, &ParentEntryID_len, &ParentEntryID);

      LOG(DEBUG) << " ParentEntryID_len = " << ParentEntryID_len << "\n";
      if (ParentEntryID_len < MIN_ENTRY_ID_LEN)
      {
         LOG_printf(DEBUG, "Warning: ParentEntryID is NULL or too small!\n");
         // return false;
      }
      LOG(DEBUG) << "----------- 4 -------\n";

      std::string ParentEntryID_String = ParentEntryID;
      // for (int i = 0; i < ParentEntryID_len; ++i) {
      //    if (ParentEntryID[i] != nullptr) { // 确保指针不是 nullptr
      //       ParentEntryID_String += ParentEntryID[i]; // 将字符串添加到 combinedString
      //       // 如果你想要在字符串之间添加分隔符，比如逗号
      //       if (i < ParentEntryID_len - 1) {
      //             ParentEntryID_String += ", "; // 添加逗号和空格作为分隔符
      //       }
      //    }
      // }

      // 现在 ParentEntryID_String 包含了所有字符串数组中的字符串
      LOG(DEBUG) << ParentEntryID_String << std::endl;
      LOG(DEBUG) << "----------- 5 -------\n";

      // 4
      unsigned EntryID_len;
      const char *EntryID = nullptr;

      Serialization_deserializeStrAlign4(&ctx, &EntryID_len, &EntryID);
      LOG(DEBUG) << " EntryID_len = " << EntryID_len << "\n";

      std::string EntryID_String = EntryID;
      // for (int i = 0; i < EntryID_len; ++i) {
      //    if (EntryID[i] != nullptr) { // 确保指针不是 nullptr
      //       EntryID_String += EntryID[i]; // 将字符串添加到 combinedString
      //       // 如果你想要在字符串之间添加分隔符，比如逗号
      //       if (i < EntryID_len - 1) {
      //             EntryID_String += ", "; // 添加逗号和空格作为分隔符
      //       }
      //    }
      // }

      // 现在 EntryID_String 包含了所有字符串数组中的字符串
      LOG(DEBUG) << EntryID_String << std::endl;
      // 5
      unsigned fileName_len;
      const char *fileName = nullptr;

      Serialization_deserializeStrAlign4(&ctx, &fileName_len, &fileName);
      LOG(DEBUG) << " fileName_len = " << fileName_len << "\n";

      std::string fileName_String = fileName;
      // for (int i = 0; i < fileName_len; ++i) {
      //    if (fileName[i] != nullptr) { // 确保指针不是 nullptr
      //       fileName_String += fileName[i]; // 将字符串添加到 combinedString
      //       // 如果你想要在字符串之间添加分隔符，比如逗号
      //       if (i < fileName_len - 1) {
      //             fileName_String += ", "; // 添加逗号和空格作为分隔符
      //       }
      //    }
      // }

      // 现在 fileName_String 包含了所有字符串数组中的字符串
      LOG(DEBUG) << fileName_String << std::endl;
      // 6
      NumNodeID ownerNodeID;
      Serialization_deserializeUInt(&ctx, &(ownerNodeID.value));
      LOG(DEBUG) << " ownerNodeID = " << ownerNodeID.value << "\n";

      // 7
      //  padding for 4-byte alignment
      uint16_t padding;
      Serialization_deserializeUShort(&ctx, &padding);
      LOG(DEBUG) << "padding  = " << padding << "\n";
      // EntryInfo_class all(ownerNodeID, ParentEntryID_String,
      // EntryID_String,fileName_String, *entryType,
      // featureFlags);

      // 后 entryDepth
      uint32_t entryDepth;
      Serialization_deserializeUInt(&ctx, &entryDepth);
      LOG(DEBUG) << " entryDepth = " << entryDepth << "\n";

      this->entryInfo.set(ownerNodeID, ParentEntryID_String,
                          // EntryID_String,fileName_String, *entryType, //$$
                          EntryID_String, fileName_String, entryType, //$$
                          featureFlags, entryDepth);

      LOG(DEBUG) << "after ctx.length = " << ctx.length << "\n";

      return true;
   }

private:
   int32_t result;

   EntryInfoWithDepth *entryInfoPtr; // for serialization
   EntryInfoWithDepth entryInfo;     // for deserialization

public:
   // inliners

   // getters & setters
   int getResult() const
   {
      return result;
   }

   EntryInfoWithDepth &getEntryInfo()
   {
      return entryInfo;
   }
};

inline void swap(Serializer &a, Serializer &b)
{
   a.swap(b);
}

// 序列化 Path
static inline void serializeDirSeparators(SerializeCtx *ctx, const std::vector<std::string::size_type> &dirSeparators)
{
   //  if (ctx->data && ctx->length < ctx->capacity) {
   if (ctx->data)
   {

      for (std::string::size_type value : dirSeparators)
      {
         // 确保在写入前检查剩余空间是否足够
         // if (ctx->length + sizeof(std::string::size_type) > ctx->capacity) {
         //     // 处理错误或超出容量的情况
         //     std::cerr << "[serializeDirSeparators] error: not enough space left in buffer\n";
         //     return; // 或者抛出异常、返回错误代码等
         // }

         uint8_t *p = (uint8_t *)ctx->data + ctx->length;
         // 按小端序写入value的每个字节
         if (sizeof(std::string::size_type) == 4)
         {
            // 32位系统
            p[0] = (value & 0xFF);
            p[1] = (value >> 8) & 0xFF;
            p[2] = (value >> 16) & 0xFF;
            p[3] = (value >> 24) & 0xFF;
         }
         else
         {
            // 64位系统
            p[0] = (value & 0xFF);
            p[1] = (value >> 8) & 0xFF;
            p[2] = (value >> 16) & 0xFF;
            p[3] = (value >> 24) & 0xFF;
            p[4] = (value >> 32) & 0xFF;
            p[5] = (value >> 40) & 0xFF;
            p[6] = (value >> 48) & 0xFF;
            p[7] = (value >> 56) & 0xFF;
         }

         ctx->length += sizeof(std::string::size_type);
      }
   }
}

// template<typename T>
// struct IsSerdesPrimitive {
//    typedef typename boost::decay<T>::type value_type;
//    enum {
//       value =
//          boost::is_same<value_type, bool>::value
//          || boost::is_same<value_type, char>::value
//          || boost::is_same<value_type, uint8_t>::value
//          || boost::is_same<value_type, int16_t>::value
//          || boost::is_same<value_type, uint16_t>::value
//          || boost::is_same<value_type, int32_t>::value
//          || boost::is_same<value_type, uint32_t>::value
//          || boost::is_same<value_type, int64_t>::value
//          || boost::is_same<value_type, uint64_t>::value
//    };
// };

static void deserializeHeader(char *buf, size_t bufLen, NetMessageHeader *outHeader)
{
   Deserializer des(buf, bufLen);
   des % *outHeader;
   if (!des.good())
      outHeader->msgType = NETMSGTYPE_Invalid;
}

#define MSGBUF_SMALL_SIZE 2048
// std::vector<char> MessagingTk::createMsgVec(NetMessage& msg)
std::vector<char> createMsgVec(NetMessage_class &msg) //@@

{

   std::vector<char> result(MSGBUF_SMALL_SIZE);

   // LOG(DEBUG)<<"-----------1-----------\n";
   auto serializeRes = msg.serializeMessage(&result[0], result.size());
   // LOG(DEBUG)<<"result.size() = "<<result.size()<<"\n";
   // for (auto it = result.begin(); it != result.end(); ++it) {
   //    LOG(DEBUG) << *it ;
   // }
   // LOG(DEBUG) << std::endl;
   LOG(DEBUG) << "-----------createMsgVec end-----------\n";

   if (!serializeRes.first)
   {
      LOG(DEBUG) << "!serializeRes.first \n";
      result.resize(serializeRes.second);
      serializeRes = msg.serializeMessage(&result[0], result.size());
   }

   result.resize(serializeRes.second);
   LOG(DEBUG) << "after result.size() = " << result.size() << "\n";

   return result;
}

//----------------------------tmp_createmsgvec.cpp -----------------end

//--------------------------- recvMsgBuf -----------------begin

// receive response---------------------------------------------------------------------

// ssize_t StandardSocket_recvfrom(StandardSocket* ssock, struct iov_iter* iter, int flags,fhgfs_sockaddr_in *from)
// ssize_t recvfrom(StandardSocket* ssock, struct iovec* iov, int flags,fhgfs_sockaddr_in *from)
ssize_t newrecvfrom(StandardSocket *ssock, void *buf, size_t len, int flags, fhgfs_sockaddr_in *from)

{
   LOG(DEBUG) << "go into   newrecvfrom \n";
   if (!ssock)
   {
      LOG_printf(DEBUG, "[newrecvfrom] error : ssock==null \n");
      return -1; // 检查空指针
   }
   int sockfd = ssock->sock;

   struct sockaddr_in fromSockAddr;
   socklen_t fromAddrLen = sizeof(fromSockAddr);
   //   ssize_t recvRes = recvfrom(sockfd, iov->iov_base, iov->iov_len, flags, (struct sockaddr*)&fromSockAddr, &fromAddrLen);
   ssize_t recvRes = recvfrom(sockfd, buf, len, flags, (struct sockaddr *)&fromSockAddr, &fromAddrLen);

   LOG(DEBUG) << "recvRes = " << recvRes << "\n";
   if (recvRes < 0)
   {
      // 处理错误
      std::cerr << "[newrecvfrom] newrecvfrom failed: " << strerror(errno) << "\n";
      return -1; // 返回错误
   }
   else if (recvRes == 0)
   {
      // 对端关闭了连接
      std::cerr << "[newrecvfrom] Connection closed by peer" << std::endl;
      return 0; // 返回 0 表示连接已关闭
   }

   if (recvRes > 0 && from)
   { // 将网络字节序转换为主机字节序
      from->addr = fromSockAddr.sin_addr;
      from->port = ntohs(fromSockAddr.sin_port);
   }
   LOG(DEBUG) << "out of   newrecvfrom \n";

   return recvRes;
}

// ssize_t StandardSocket_recvfromT(StandardSocket* thissock, struct iov_iter* iter, int flags,fhgfs_sockaddr_in *from, int timeoutMS)
// ssize_t recvfromT(StandardSocket* thissock, struct iovec* iov, int flags,fhgfs_sockaddr_in *from, int timeoutMS)
ssize_t recvfromT(StandardSocket *thissock, void *buf, size_t len, int flags, fhgfs_sockaddr_in *from, int timeoutMS)
{

   LOG(DEBUG) << "go into recvfromT \n";

   struct pollfd pollfd;
   pollfd.fd = thissock->sock;
   pollfd.events = POLLIN;

   // 设置 socket 超时时间
   struct timeval tv;
   tv.tv_sec = timeoutMS / 1000;           // 秒
   tv.tv_usec = (timeoutMS % 1000) * 1000; // 微秒
   if (setsockopt(thissock->sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
   {
      perror("setsockopt failed");
      return -1;
   }

   int pollRes = ::poll(&pollfd, 1, timeoutMS); // pollRes 是 poll 系统调用的返回值，它表示有多少个文件描述符（在本例中是 socket）已经准备好了 I/O 操作。
   if (pollRes > 0 && (pollfd.revents & POLLIN))
   { // 检查 pollfd.revents 是否包含 POLLIN 事件，这表示 socket 上有数据可读。

      LOG_printf(DEBUG, "[recvfromT]  pollRes > 0 && (pollfd.revents & POLLIN) \n");
      ssize_t res = newrecvfrom(thissock, buf, len, flags, from);
      LOG(DEBUG) << "res from newrecvfrom= " << res << "\n";
      return res;
      // return recvfrom(thissock,iov, flags, from);   //如果这两个条件都满足，说明 socket 已经准备好接收数据，函数将返回 recvfrom 函数的调用结果，该结果可能是接收到的数据字节数，或者是错误码（如果是负数）。
   }
   else if (pollRes == 0)
   { // 这意味着 poll 系统调用超时了，因为在指定的超时时间内没有文件描述符准备好 I/O 操作。
      errno = ETIMEDOUT;
      LOG_printf(DEBUG, "[recvfromT] error: pollRes == 0 超时错误\n");
      return -1;
   }
   else
   {
      // Error handling
      // std::cerr << "recvfromT: poll() failed with error: " << strerror(errno) << std::endl;
      LOG_printf(DEBUG, "[recvfromT] error: poll() failed with error: %s\n", strerror(errno));

      return -1;
   }
   LOG(DEBUG) << "out of recvfromT \n";
}

// ssize_t StandardSocket::recvT(void *buf, size_t len, int flags, int timeoutMS)
ssize_t recvT(Socket *thissock, void *buf, size_t len, int flags, int timeoutMS)
{
   LOG(DEBUG) << "go into recvT \n";
   StandardSocket *thisCast = (StandardSocket *)thissock;
   // ssize_t res= recvfromT(thisCast, iter, flags, nullptr, timeoutMS);
   ssize_t res = recvfromT(thisCast, buf, len, flags, nullptr, timeoutMS);

   LOG(DEBUG) << "out of recvT \n";

   return res;
}

/**
 * @throw SocketException
 */
//   inline ssize_t recvExactT(void *buf, size_t len, int flags, int timeoutMS)
inline ssize_t recvExactT(Socket *thissock, void *buf, size_t len, int flags, int timeoutMS)

{
   // note: this uses a soft timeout that is being reset after each received chunk
   LOG(DEBUG) << "go into recvExactT  \n";
   ssize_t missing = len;
   LOG(DEBUG) << "missing = " << missing << "\n";

   do
   {
      ssize_t recvRes = recvT(thissock, &((char *)buf)[len - missing], missing, flags, timeoutMS);
      LOG(DEBUG) << " recvRes = " << recvRes << "\n";
      missing -= recvRes;
   } while (missing);
   LOG(DEBUG) << "out of recvExactT  \n";

   return (ssize_t)len;
}

// 反序列化    -------------------------------------------------------------------
#define RECEIVE_TIMEOUT 600000

#define MSGBUF_DEFAULT_SIZE (64 * 1024)   // at least big enough to store a datagram
#define MSGBUF_MAX_SIZE (4 * 1024 * 1024) // max accepted size

static unsigned extractMsgLengthFromBuf(const char *recvBuf, unsigned bufLen)
{
   LOG(DEBUG) << "----------------1 -------------\n";

   Deserializer des(recvBuf, bufLen);
   LOG(DEBUG) << "----------------2 -------------\n";

   uint32_t length;
   LOG(DEBUG) << "length = " << length << "\n";
   LOG(DEBUG) << "----------------31 -------------\n";

   des % length;
   LOG(DEBUG) << "----------------3 -------------\n";

   LOG(DEBUG) << "des.good = " << des.good() << "\t, length = " << length << "\n";
   // return -1 if the buffer was short, which will also be larger than any possible message
   // we can possibly receive (due to 16bit message length)
   return des.good() ? length : -1;
}

// std::vector<char> MessagingTk::recvMsgBuf(Socket& socket, int minTimeout)
std::vector<char> recvMsgBuf(Socket &socket, int minTimeout)
try
{
   // AbstractApp* app = PThread::getCurrentThreadApp();
   // int connMsgLongTimeout = app->getCommonConfig()->getConnMsgLongTimeout();

   // DEBUG_ENV_VAR(unsigned, RECEIVE_TIMEOUT, connMsgLongTimeout, "BEEGFS_MESSAGING_RECV_TIMEOUT_MS");

   // 只是保留以下

   const int recvTimeoutMS = minTimeout < 0
                                 ? -1
                                 : std::max<int>(minTimeout, RECEIVE_TIMEOUT); //@@

   std::vector<char> result(MSGBUF_DEFAULT_SIZE); // 64KB

   // receive at least the message header

   // unsigned numReceived = socket.recvExactT(&result[0], NETMSG_MIN_LENGTH, 0, recvTimeoutMS);
   unsigned numReceived = recvExactT(&socket, &result[0], NETMSG_MIN_LENGTH, 0, recvTimeoutMS); // 40B

   // LOG(DEBUG)<<" 1 out of recvExactT \n";
   // for(int i=0;i<numReceived;i++)
   // {
   //    LOG(DEBUG)<<result[i]<<" ";
   // }
   // LOG(DEBUG)<<"numReceived = "<<numReceived<<"\n";
   // unsigned msgLength = NetMessageHeader::extractMsgLengthFromBuf(&result[0], numReceived);
   unsigned msgLength = extractMsgLengthFromBuf(&result[0], numReceived);
   LOG(DEBUG) << " msgLength  = " << msgLength << "\n";
   LOG(DEBUG) << "----------------1 -------------\n";
   if (msgLength > MSGBUF_MAX_SIZE)
   {
      // LOG(COMMUNICATION, ERR, "Received a message with invalid length.",
      //       ("from", socket.getPeername()));
      LOG(DEBUG) << "Received a message with invalid length. \n";
      return {};
   }
   LOG(DEBUG) << "----------------2 -------------\n";

   result.resize(msgLength);

   // receive the rest of the message

   if (msgLength > numReceived)
      // socket.recvExactT(&result[numReceived], msgLength-numReceived, 0, recvTimeoutMS);
      recvExactT(&socket, &result[numReceived], msgLength - numReceived, 0, recvTimeoutMS);

   LOG(DEBUG) << " 2 out of recvExactT \n";

   return result;
}
catch (const std::bad_alloc &)
{
   return {};
}

//---------------------------- recvMsgBuf -----------------end

// 反序列化 ---------------------------begin

class SimpleMsg : public NetMessage_class
{
public:
   SimpleMsg(unsigned short msgType) : NetMessage_class(msgType)
   {
   }

   // protected:
   virtual void serializePayload(Serializer &ser) const
   {
      // nothing to be done here for simple messages
   }

   virtual bool deserializePayload(const char *buf, size_t bufLen)
   {
      // nothing to be done here for simple messages
      return true;
   }
};

/**
 * @return NetMessage that must be deleted by the caller
 * (msg->msgType is NETMSGTYPE_Invalid on error)
 */
// std::unique_ptr<NetMessage> NetMessageFactory::createFromMsgType(unsigned short msgType) const
std::unique_ptr<NetMessage_class> createFromMsgType(unsigned short msgType)

{
   NetMessage_class *msg;

   switch (msgType)
   {
   // The following lines are grouped by "type of the message" and ordered alphabetically inside
   // the groups. There should always be one message per line to keep a clear layout (although
   // this might lead to lines that are longer than usual)
   // case NETMSGTYPE_FindOwner: { msg = new FindOwnerMsgEx(); } break;
   case NETMSGTYPE_FindOwnerResp:
   {
      msg = new FindOwnerRespMsg();
   }
   break;
   default:
   {
      msg = new SimpleMsg(NETMSGTYPE_Invalid);
      LOG(DEBUG) << "[createFromMsgType] error type is not NETMSGTYPE_FindOwner \n";
   }
   break;
      /*
         // control messages
         case NETMSGTYPE_Ack: { msg = new AckMsgEx(); } break;
         case NETMSGTYPE_AuthenticateChannel: { msg = new AuthenticateChannelMsgEx(); } break;
         case NETMSGTYPE_GenericResponse: { msg = new GenericResponseMsg(); } break;
         case NETMSGTYPE_SetChannelDirect: { msg = new SetChannelDirectMsgEx(); } break;
         case NETMSGTYPE_PeerInfo: { msg = new PeerInfoMsgEx(); } break;

         // nodes messages
         case NETMSGTYPE_ChangeTargetConsistencyStatesResp: { msg = new ChangeTargetConsistencyStatesRespMsg(); } break;
         case NETMSGTYPE_GenericDebug: { msg = new GenericDebugMsgEx(); } break;
         case NETMSGTYPE_GetClientStats: { msg = new GetClientStatsMsgEx(); } break;
         case NETMSGTYPE_GetMirrorBuddyGroupsResp: { msg = new GetMirrorBuddyGroupsRespMsg(); } break;
         case NETMSGTYPE_GetNodeCapacityPools: { msg = new GetNodeCapacityPoolsMsgEx(); } break;
         case NETMSGTYPE_GetNodeCapacityPoolsResp: { msg = new GetNodeCapacityPoolsRespMsg(); } break;
         case NETMSGTYPE_GetNodes: { msg = new GetNodesMsgEx(); } break;
         case NETMSGTYPE_GetNodesResp: { msg = new GetNodesRespMsg(); } break;
         case NETMSGTYPE_GetStatesAndBuddyGroupsResp: { msg = new GetStatesAndBuddyGroupsRespMsg(); } break;
         case NETMSGTYPE_GetStoragePoolsResp: { msg = new GetStoragePoolsRespMsg(); } break;
         case NETMSGTYPE_GetTargetMappings: { msg = new GetTargetMappingsMsgEx(); } break;
         case NETMSGTYPE_GetTargetMappingsResp: { msg = new GetTargetMappingsRespMsg(); } break;
         case NETMSGTYPE_GetTargetStatesResp: { msg = new GetTargetStatesRespMsg(); } break;
         case NETMSGTYPE_HeartbeatRequest: { msg = new HeartbeatRequestMsgEx(); } break;
         case NETMSGTYPE_Heartbeat: { msg = new HeartbeatMsgEx(); } break;
         case NETMSGTYPE_MapTargets: { msg = new MapTargetsMsgEx(); } break;
         case NETMSGTYPE_PublishCapacities: { msg = new PublishCapacitiesMsgEx(); } break;
         case NETMSGTYPE_RefreshStoragePools: { msg = new RefreshStoragePoolsMsgEx(); } break;
         case NETMSGTYPE_RegisterNodeResp: { msg = new RegisterNodeRespMsg(); } break;
         case NETMSGTYPE_RemoveNode: { msg = new RemoveNodeMsgEx(); } break;
         case NETMSGTYPE_RemoveNodeResp: { msg = new RemoveNodeRespMsg(); } break;
         case NETMSGTYPE_RefreshCapacityPools: { msg = new RefreshCapacityPoolsMsgEx(); } break;
         case NETMSGTYPE_RefreshTargetStates: { msg = new RefreshTargetStatesMsgEx(); } break;
         case NETMSGTYPE_SetMirrorBuddyGroup: { msg = new SetMirrorBuddyGroupMsgEx(); } break;
         case NETMSGTYPE_SetTargetConsistencyStates: { msg = new SetTargetConsistencyStatesMsgEx(); } break;
         case NETMSGTYPE_SetTargetConsistencyStatesResp: { msg = new SetTargetConsistencyStatesRespMsg(); } break;

         // storage messages
         case NETMSGTYPE_FindLinkOwner: { msg = new FindLinkOwnerMsgEx(); } break;
         case NETMSGTYPE_FindOwner: { msg = new FindOwnerMsgEx(); } break;
         case NETMSGTYPE_FindOwnerResp: { msg = new FindOwnerRespMsg(); } break;
         case NETMSGTYPE_GetChunkFileAttribsResp: { msg = new GetChunkFileAttribsRespMsg(); } break;
         case NETMSGTYPE_GetEntryInfo: { msg = new GetEntryInfoMsgEx(); } break;
         case NETMSGTYPE_GetEntryInfoResp: { msg = new GetEntryInfoRespMsg(); } break;
         case NETMSGTYPE_GetHighResStats: { msg = new GetHighResStatsMsgEx(); } break;
         case NETMSGTYPE_GetMetaResyncStats: { msg = new GetMetaResyncStatsMsgEx(); } break;
         case NETMSGTYPE_RequestExceededQuotaResp: {msg = new RequestExceededQuotaRespMsg(); } break;
         case NETMSGTYPE_SetExceededQuota: {msg = new SetExceededQuotaMsgEx(); } break;
         case NETMSGTYPE_StorageResyncStarted: { msg = new StorageResyncStartedMsgEx(); } break;
         case NETMSGTYPE_StorageResyncStartedResp: { msg = new StorageResyncStartedRespMsg(); } break;
         case NETMSGTYPE_GetXAttr: { msg = new GetXAttrMsgEx(); } break;
         case NETMSGTYPE_GetXAttrResp: { msg = new GetXAttrRespMsg(); } break;
         case NETMSGTYPE_Hardlink: { msg = new HardlinkMsgEx(); } break;
         case NETMSGTYPE_HardlinkResp: { msg = new HardlinkRespMsg(); } break;
         case NETMSGTYPE_ListDirFromOffset: { msg = new ListDirFromOffsetMsgEx(); } break;
         case NETMSGTYPE_ListDirFromOffsetResp: { msg = new ListDirFromOffsetRespMsg(); } break;
         case NETMSGTYPE_ListXAttr: { msg = new ListXAttrMsgEx(); } break;
         case NETMSGTYPE_ListXAttrResp: { msg = new ListXAttrRespMsg(); } break;
         case NETMSGTYPE_LookupIntent: { msg = new LookupIntentMsgEx(); } break;
         case NETMSGTYPE_LookupIntentResp: { msg = new LookupIntentRespMsg(); } break;
         case NETMSGTYPE_MkDir: { msg = new MkDirMsgEx(); } break;
         case NETMSGTYPE_MkDirResp: { msg = new MkDirRespMsg(); } break;
         case NETMSGTYPE_MkFile: { msg = new MkFileMsgEx(); } break;
         case NETMSGTYPE_MkFileResp: { msg = new MkFileRespMsg(); } break;
         case NETMSGTYPE_MkFileWithPattern: { msg = new MkFileWithPatternMsgEx(); } break;
         case NETMSGTYPE_MkFileWithPatternResp: { msg = new MkFileWithPatternRespMsg(); } break;
         case NETMSGTYPE_MkLocalDir: { msg = new MkLocalDirMsgEx(); } break;
         case NETMSGTYPE_MkLocalDirResp: { msg = new MkLocalDirRespMsg(); } break;
         case NETMSGTYPE_MovingDirInsert: { msg = new MovingDirInsertMsgEx(); } break;
         case NETMSGTYPE_MovingDirInsertResp: { msg = new MovingDirInsertRespMsg(); } break;
         case NETMSGTYPE_MovingFileInsert: { msg = new MovingFileInsertMsgEx(); } break;
         case NETMSGTYPE_MovingFileInsertResp: { msg = new MovingFileInsertRespMsg(); } break;
         case NETMSGTYPE_RefreshEntryInfo: { msg = new RefreshEntryInfoMsgEx(); } break;
         case NETMSGTYPE_RefreshEntryInfoResp: { msg = new RefreshEntryInfoRespMsg(); } break;
         case NETMSGTYPE_ResyncRawInodes: { msg = new ResyncRawInodesMsgEx(); } break;
         case NETMSGTYPE_ResyncRawInodesResp: { msg = new ResyncRawInodesRespMsg(); } break;
         case NETMSGTYPE_ResyncSessionStore: { msg = new ResyncSessionStoreMsgEx(); } break;
         case NETMSGTYPE_ResyncSessionStoreResp: { msg = new ResyncSessionStoreRespMsg(); } break;
         case NETMSGTYPE_RemoveXAttr: { msg = new RemoveXAttrMsgEx(); } break;
         case NETMSGTYPE_RemoveXAttrResp: { msg = new RemoveXAttrRespMsg(); } break;
         case NETMSGTYPE_Rename: { msg = new RenameV2MsgEx(); } break;
         case NETMSGTYPE_RenameResp: { msg = new RenameRespMsg(); } break;
         case NETMSGTYPE_RmChunkPathsResp: { msg = new RmChunkPathsRespMsg(); } break;
         case NETMSGTYPE_RmDirEntry: { msg = new RmDirEntryMsgEx(); } break;
         case NETMSGTYPE_RmDir: { msg = new RmDirMsgEx(); } break;
         case NETMSGTYPE_RmDirResp: { msg = new RmDirRespMsg(); } break;
         case NETMSGTYPE_RmLocalDir: { msg = new RmLocalDirMsgEx(); } break;
         case NETMSGTYPE_RmLocalDirResp: { msg = new RmLocalDirRespMsg(); } break;
         case NETMSGTYPE_SetAttr: { msg = new SetAttrMsgEx(); } break;
         case NETMSGTYPE_SetAttrResp: { msg = new SetAttrRespMsg(); } break;
         case NETMSGTYPE_SetDirPattern: { msg = new SetDirPatternMsgEx(); } break;
         case NETMSGTYPE_SetDirPatternResp: { msg = new SetDirPatternRespMsg(); } break;
         case NETMSGTYPE_SetLocalAttrResp: { msg = new SetLocalAttrRespMsg(); } break;
         case NETMSGTYPE_SetMetadataMirroring: { msg = new SetMetadataMirroringMsgEx(); } break;
         case NETMSGTYPE_SetStorageTargetInfoResp: { msg = new SetStorageTargetInfoRespMsg(); } break;
         case NETMSGTYPE_SetXAttr: { msg = new SetXAttrMsgEx(); } break;
         case NETMSGTYPE_SetXAttrResp: { msg = new SetXAttrRespMsg(); } break;
         case NETMSGTYPE_Stat: { msg = new StatMsgEx(); } break;
         case NETMSGTYPE_StatResp: { msg = new StatRespMsg(); } break;
         case NETMSGTYPE_StatStoragePath: { msg = new StatStoragePathMsgEx(); } break;
         case NETMSGTYPE_StatStoragePathResp: { msg = new StatStoragePathRespMsg(); } break;
         case NETMSGTYPE_TruncFile: { msg = new TruncFileMsgEx(); } break;
         case NETMSGTYPE_TruncFileResp: { msg = new TruncFileRespMsg(); } break;
         case NETMSGTYPE_TruncLocalFileResp: { msg = new TruncLocalFileRespMsg(); } break;
         case NETMSGTYPE_UnlinkFile: { msg = new UnlinkFileMsgEx(); } break;
         case NETMSGTYPE_UnlinkFileResp: { msg = new UnlinkFileRespMsg(); } break;
         case NETMSGTYPE_UnlinkLocalFileResp: { msg = new UnlinkLocalFileRespMsg(); } break;
         case NETMSGTYPE_UpdateDirParent: { msg = new UpdateDirParentMsgEx(); } break;
         case NETMSGTYPE_UpdateDirParentResp: { msg = new UpdateDirParentRespMsg(); } break;
         case NETMSGTYPE_MoveFileInode: { msg = new MoveFileInodeMsgEx(); } break;
         case NETMSGTYPE_MoveFileInodeResp: {msg = new MoveFileInodeRespMsg(); } break;
         case NETMSGTYPE_UnlinkLocalFileInode: {msg = new UnlinkLocalFileInodeMsgEx(); } break;
         case NETMSGTYPE_UnlinkLocalFileInodeResp: {msg = new UnlinkLocalFileInodeRespMsg(); } break;

         // session messages
         case NETMSGTYPE_BumpFileVersion: { msg = new BumpFileVersionMsgEx(); } break;
         case NETMSGTYPE_BumpFileVersionResp: { msg = new BumpFileVersionRespMsg(); } break;
         case NETMSGTYPE_OpenFile: { msg = new OpenFileMsgEx(); } break;
         case NETMSGTYPE_OpenFileResp: { msg = new OpenFileRespMsg(); } break;
         case NETMSGTYPE_CloseFile: { msg = new CloseFileMsgEx(); } break;
         case NETMSGTYPE_CloseFileResp: { msg = new CloseFileRespMsg(); } break;
         case NETMSGTYPE_CloseChunkFileResp: { msg = new CloseChunkFileRespMsg(); } break;
         case NETMSGTYPE_WriteLocalFileResp: { msg = new WriteLocalFileRespMsg(); } break;
         case NETMSGTYPE_FSyncLocalFileResp: { msg = new FSyncLocalFileRespMsg(); } break;
         case NETMSGTYPE_FLockAppend: { msg = new FLockAppendMsgEx(); } break;
         case NETMSGTYPE_FLockAppendResp: { msg = new FLockAppendRespMsg(); } break;
         case NETMSGTYPE_FLockEntry: { msg = new FLockEntryMsgEx(); } break;
         case NETMSGTYPE_FLockEntryResp: { msg = new FLockEntryRespMsg(); } break;
         case NETMSGTYPE_FLockRange: { msg = new FLockRangeMsgEx(); } break;
         case NETMSGTYPE_FLockRangeResp: { msg = new FLockRangeRespMsg(); } break;
         case NETMSGTYPE_GetFileVersion: { msg = new GetFileVersionMsgEx(); } break;
         case NETMSGTYPE_GetFileVersionResp: { msg = new GetFileVersionRespMsg(); } break;
         case NETMSGTYPE_AckNotify: { msg = new AckNotifiyMsgEx(); } break;
         case NETMSGTYPE_AckNotifyResp: { msg = new AckNotifiyRespMsg(); } break;

         // mon message
         case NETMSGTYPE_RequestMetaData: { msg = new RequestMetaDataMsgEx(); } break;

         // fsck messages
         case NETMSGTYPE_RetrieveDirEntries: { msg = new RetrieveDirEntriesMsgEx(); } break;
         case NETMSGTYPE_RetrieveInodes: { msg = new RetrieveInodesMsgEx(); } break;
         case NETMSGTYPE_RetrieveFsIDs: { msg = new RetrieveFsIDsMsgEx(); } break;
         case NETMSGTYPE_DeleteDirEntries: { msg = new DeleteDirEntriesMsgEx(); } break;
         case NETMSGTYPE_CreateDefDirInodes: { msg = new CreateDefDirInodesMsgEx(); } break;
         case NETMSGTYPE_FixInodeOwners: { msg = new FixInodeOwnersMsgEx(); } break;
         case NETMSGTYPE_FixInodeOwnersInDentry: { msg = new FixInodeOwnersInDentryMsgEx(); } break;
         case NETMSGTYPE_LinkToLostAndFound: { msg = new LinkToLostAndFoundMsgEx(); } break;
         case NETMSGTYPE_CreateEmptyContDirs: { msg = new CreateEmptyContDirsMsgEx(); } break;
         case NETMSGTYPE_UpdateFileAttribs: { msg = new UpdateFileAttribsMsgEx(); } break;
         case NETMSGTYPE_UpdateDirAttribs: { msg = new UpdateDirAttribsMsgEx(); } break;
         case NETMSGTYPE_RemoveInodes: { msg = new RemoveInodesMsgEx(); } break;
         case NETMSGTYPE_RecreateFsIDs: { msg = new RecreateFsIDsMsgEx(); } break;
         case NETMSGTYPE_RecreateDentries: { msg = new RecreateDentriesMsgEx(); } break;
         case NETMSGTYPE_FsckSetEventLogging: { msg = new FsckSetEventLoggingMsgEx(); } break;
         case NETMSGTYPE_AdjustChunkPermissions: { msg = new AdjustChunkPermissionsMsgEx(); } break;
         case NETMSGTYPE_CheckAndRepairDupInode: { msg = new CheckAndRepairDupInodeMsgEx(); } break;

         default:
         {
            msg = new SimpleMsg(NETMSGTYPE_Invalid);
         } break;
      */
   }

   return std::unique_ptr<NetMessage_class>(msg);
}

/**
 * Create NetMessage object (specific type determined by msg header) from a raw msg payload buffer,
 * for which the msg header has already been deserialized.
 *
 * @return (msg->msgType is NETMSGTYPE_Invalid on error)
 */
// std::unique_ptr<NetMessage> AbstractNetMessageFactory::createFromPreprocessedBuf(
//       NetMessageHeader* header, char* msgPayloadBuf, size_t msgPayloadBufLen) const
std::unique_ptr<NetMessage_class> createFromPreprocessedBuf(
    NetMessageHeader *header, char *msgPayloadBuf, size_t msgPayloadBufLen)
{
   const char *logContext = "NetMsgFactory (create msg from buf)";

   // create the message object for the given message type

   std::unique_ptr<NetMessage_class> msg(createFromMsgType(header->msgType));
   if (msg->getmsgType() == NETMSGTYPE_Invalid)
   {
      LOG(DEBUG) << "Received an invalid or unhandled message.  \n";
      // LogContext(logContext).log(Log_NOTICE,
      //    "Received an invalid or unhandled message. "
      //    "Message type (from raw header): " + netMessageTypeToStr(header->msgType));

      return msg;
   }

   // apply message feature flags and check compatibility

   msg->setMsgHeaderFeatureFlags(header->msgFeatureFlags);

   bool checkCompatRes = msg->checkHeaderFeatureFlagsCompat();
   if (!checkCompatRes)
   { // incompatible feature flag was set => log error with msg type
      LOG(DEBUG) << "Received a message with incompatible feature flags. \n";
      // LogContext(logContext).log(Log_WARNING,
      //    "Received a message with incompatible feature flags. "
      //    "Message type: " + netMessageTypeToStr(header->msgType) + "; "
      //    "Flags (hex): " + StringTk::uintToHexStr(msg->getMsgHeaderFeatureFlags() ) );

      // return std::make_unique<SimpleMsg>(NETMSGTYPE_Invalid);
      return std::unique_ptr<SimpleMsg>(new SimpleMsg(NETMSGTYPE_Invalid));
   }

   // check whether the header flags are as we expect them:
   //  * if the message does not support mirroring, header flags must be 0
   //  * otherwise, they must not contain flags that are not defined
   uint8_t FlagsMask = 0x07;
   // if (header->msgFlags & ~(msg->supportsMirroring() ? NetMessageHeader::FlagsMask : 0))
   if (header->msgFlags & ~(msg->supportsMirroring() ? FlagsMask : 0))

   {
      LOG(DEBUG) << "Received a message with invalid header flags \n";
      // LOG(GENERAL, WARNING, "Received a message with invalid header flags", header->msgType,
      //       header->msgFlags);

      // return std::make_unique<SimpleMsg>(NETMSGTYPE_Invalid);
      return std::unique_ptr<SimpleMsg>(new SimpleMsg(NETMSGTYPE_Invalid));
   }

   msg->getmsgHeader() = *header;

   // deserialize message payload

   bool deserRes = msg->deserializePayload(msgPayloadBuf, msgPayloadBufLen); ////@@------3--------- 进入这里 反序列化失败导致的问题
   if (!deserRes)
   { // deserialization failed => log error with msg type
      LOG(DEBUG) << "Failed to decode message.  \n";
      // LogContext(logContext).log(Log_NOTICE,
      //    "Failed to decode message. "
      //    "Message type: " + netMessageTypeToStr(header->msgType));

      // return std::make_unique<SimpleMsg>(NETMSGTYPE_Invalid);
      return std::unique_ptr<SimpleMsg>(new SimpleMsg(NETMSGTYPE_Invalid));
   }

   return msg;
}

/**
 * Create NetMessage object (specific type determined by msg header) from a raw msg buffer.
 *
 * @return (msg->msgType is NETMSGTYPE_Invalid on error)
 */
// std::unique_ptr<NetMessage> AbstractNetMessageFactory::createFromRaw(char* recvBuf,size_t bufLen) const
std::unique_ptr<NetMessage_class> createFromRaw(char *recvBuf, size_t bufLen)

{
   if (bufLen < NETMSG_MIN_LENGTH)
   {
      // return std::make_unique<SimpleMsg>(NETMSGTYPE_Invalid);
      LOG(DEBUG) << "bufLen < NETMSG_MIN_LENGTH \n";
      return std::unique_ptr<SimpleMsg>(new SimpleMsg(NETMSGTYPE_Invalid));
   }
   NetMessageHeader header;

   // decode the message header
   NetMessage_class::deserializeHeader(recvBuf, bufLen, &header);

   // delegate the rest of the work to another method...

   char *msgPayloadBuf = recvBuf + NETMSG_HEADER_LENGTH;
   size_t msgPayloadBufLen = bufLen - NETMSG_HEADER_LENGTH;

   return createFromPreprocessedBuf(&header, msgPayloadBuf, msgPayloadBufLen);
}

std::unique_ptr<NetMessage_class> createFromBuf(std::vector<char> buf)
{
   auto result = createFromRaw(&buf[0], buf.size());
   if (result)
      result->getbackingBuffer() = std::move(buf);
   return result;
}

// 反序列化 ---------------------------end
/**
 * Sends a request message to a node and receives the response.
 *
 * @param rrArgs:
 *    .node receiver
 *    .requestMsg the message that should be sent to the receiver
 *    .respMsgType expected response message type
 *    .outRespMsg response message if successful (must be deleted by the caller)
 * @return FhgfsOpsErr_COMMUNICATION on comm error, FhgfsOpsErr_WOULDBLOCK if remote side
 *    encountered an indirect comm error and suggests not to try again, FhgfsOpsErr_AGAIN if other
 *    side is suggesting infinite retries.
 */
// FhgfsOpsErr MessagingTk::requestResponseComm(RequestResponseArgs* rrArgs)
FhgfsOpsErr requestResponseComm(RequestResponseArgs *rrArgs)

{
   // const char* logContext = "Messaging (RPC)";
   // const Node& node = *rrArgs->node;
   // NodeConnPool* connPool = node.getConnPool();

   FhgfsOpsErr retVal = FhgfsOpsErr_INTERNAL;
   Socket *sock = nullptr;

   // try
   // {
   // connect
   // sock = connPool->acquireStreamSocket();

   LOG_printf(DEBUG, "--------- 1. 建立套接字 -------------\n");
   sock = (Socket *)StandardSocket_construct(PF_INET, SOCK_STREAM, 0);
   if (!sock) // 如果获取套接字失败，记录错误并返回通信错误。
   {
      LOG_printf(DEBUG, " 获取套接字失败 \n");
      return FhgfsOpsErr_COMMUNICATION; // 随便指定了一个错误类型
   }
   else
   {
      LOG_printf(DEBUG, " 获取套接字成功 \n");
   }

   // 2. 尝试连接
   LOG_printf(DEBUG, "--------- 2. 设置TCP 缓冲区大小-------------\n");

   int tcpbufLen;
   socklen_t tcpbufLenSize = sizeof(tcpbufLen);
   if (getsockopt(((StandardSocket *)sock)->sock, SOL_SOCKET, SO_RCVBUF, &tcpbufLen, &tcpbufLenSize) < 0)
   {
      std::cerr << "Failed to get socket receive buffer size" << std::endl;
      return FhgfsOpsErr_COMMUNICATION;
   }
   else
      LOG(DEBUG) << "get socket receive buffer size = " << tcpbufLen << "\n";

   int bufSize = tcpbufLen; // 8192*70;
   if (bufSize > 0)
   {
      int val = bufSize;
      if (setsockopt(((StandardSocket *)sock)->sock, SOL_SOCKET, SO_RCVBUF, &val, sizeof(val)) < 0)
      {
         std::cerr << "Failed to set socket receive buffer size" << std::endl;
         return FhgfsOpsErr_COMMUNICATION;
      }
      else
         LOG(DEBUG) << "Succesfully set socket receive buffer size = " << val << "\n";
   }

   // 3. 尝试通过IP连接
   LOG_printf(DEBUG, "------------------- 3. 尝试通过IP和端口建立连接-------------------\n");
   unsigned short port = 8005; //@@
   NicAddress nicAddr[10];
   initNicAddressArray_meta(nicAddr); //@@
   bool connectRes;
   connectRes = sock->ops->connectByIP(sock, nicAddr[4].ipAddr, port); //@@ 224: nicAddr[8] ;162 :  nicAddr[4]
   if (connectRes)
   {
      LOG_printf(DEBUG, "3. 尝试通过IP连接 连接成功 \n");
   }
   else
   {
      LOG_printf(DEBUG, "3. 尝试通过IP连接 连接失败 \n");
   }

   // 3.1 启用TCP Keepalive
   int opt = 1;
   if (setsockopt(((StandardSocket *)sock)->sock, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt)) < 0)
   {
      perror("setsockopt failed");
      exit(EXIT_FAILURE);
   }

   // 设置TCP Keepalive参数
   LOG_printf(DEBUG, "------------------- 3.1 设置TCP Keepalive -------------------\n");

   int keepalive_time = 30;  // 30秒
   int keepalive_intvl = 5;  // 每5秒发送一次Keepalive
   int keepalive_probes = 3; // 最多发送3次Keepalive

   if (setsockopt(((StandardSocket *)sock)->sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepalive_time, sizeof(keepalive_time)) < 0)
   {
      perror("setsockopt TCP_KEEPIDLE failed");
      exit(EXIT_FAILURE);
   }

   if (setsockopt(((StandardSocket *)sock)->sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepalive_intvl, sizeof(keepalive_intvl)) < 0)
   {
      perror("setsockopt TCP_KEEPINTVL failed");
      exit(EXIT_FAILURE);
   }

   if (setsockopt(((StandardSocket *)sock)->sock, IPPROTO_TCP, TCP_KEEPCNT, &keepalive_probes, sizeof(keepalive_probes)) < 0)
   {
      perror("setsockopt TCP_KEEPCNT failed");
      exit(EXIT_FAILURE);
   }

   // 现在，TCP连接将保持活跃状态
   LOG_printf(DEBUG, "Connection established and Keepalive is enabled.\n");

   // 这是 服务端的认证配置
   LOG_printf(DEBUG, "------------------- 3.2 服务端的认证配置 -------------------\n");

   bool isflag = __NodeConnPool_applySocketOptionsConnected(sock);
   if (isflag == true)
   {
      LOG(DEBUG) << "__NodeConnPool_applySocketOptionsConnected 成功 ！！\n";
   }
   else
   {
      LOG(DEBUG) << "__NodeConnPool_applySocketOptionsConnected 失败 ！！\n";
   }

   LOG_printf(DEBUG, "--------- 4. 准备发送缓冲区 -------------\n");

   const auto sendBuf = createMsgVec(*rrArgs->requestMsg);
   // LOG(DEBUG)<<"sendBuf.size() ="<<sendBuf.size();

   LOG_printf(DEBUG, "--------- 5. 发送消息 -------------\n");
   StandardSocket *standard_sock = (StandardSocket *)sock;
   // size_t len = sendBuf.size();
   // sock->send(&sendBuf[0], sendBuf.size(), 0);// ssize_t StandardSocket::send(const void *buf, size_t len, int flags)
   LOG(DEBUG) << "sendBuf.size() = " << sendBuf.size() << "\n";
   // LOG(DEBUG)<<""<<
   int sendres = send(standard_sock->sock, &sendBuf[0], sendBuf.size(), 0); //
   if (sendres < 0)
   {
      LOG(DEBUG) << " send failed \n";
   }

   LOG(DEBUG) << "sendres = " << sendres << "\n"; // 确实是发送了102B

   if (rrArgs->sendExtraData)
   {
      LOG(DEBUG) << "还需要发送别的数据 \n";
      retVal = rrArgs->sendExtraData(sock, rrArgs->extraDataContext);
      if (retVal != FhgfsOpsErr_SUCCESS)
      // goto err_cleanup;
      {
         LOG_printf(DEBUG, "发送数据失败  \n");
      }
   }

   LOG_printf(DEBUG, "--------------------6. 接收消息-------------------\n");

   // receive response
   // auto respBuf = MessagingTk::recvMsgBuf(*sock, rrArgs->minTimeoutMS);
   auto respBuf = recvMsgBuf(*sock, rrArgs->minTimeoutMS);

   if (respBuf.empty())
   { // error (e.g. message too big)
     // LogContext(logContext).log(Log_WARNING,
     //    "Failed to receive response from: " + node.getNodeIDWithTypeStr() + "; " +
     //    sock->getPeername() + ". " +
     //    "(Message type: " + rrArgs->requestMsg->getMsgTypeStr() + ")");

      LOG(DEBUG) << "Failed to receive response ,respBuf.empty() ==true \n";
      retVal = FhgfsOpsErr_COMMUNICATION;
      // goto err_cleanup;
   }
   else
      LOG(DEBUG) << "successfully receive response ,respBuf.empty() ==false \n";

   LOG_printf(DEBUG, "--------------------7. 处理分析消息-------------------\n");

   // auto netMessageFactory = PThread::getCurrentThreadApp()->getNetMessageFactory();

   // got response => deserialize it
   rrArgs->outRespMsg = createFromBuf(std::move(respBuf));

   if (rrArgs->outRespMsg->getmsgType() == NETMSGTYPE_GenericResponse)
   { // special control msg received
      LOG(DEBUG) << "special control msg received \n";
      // retVal = handleGenericResponse(rrArgs);
      // if(retVal != FhgfsOpsErr_INTERNAL)
      // { // we can re-use the connection
      //    connPool->releaseStreamSocket(sock);
      //    sock = NULL;
      // }

      // goto err_cleanup;
   }

   if (rrArgs->outRespMsg->getmsgType() != rrArgs->respMsgType)
   { // response invalid (wrong msgType)
      LOG(DEBUG) << "response invalid (wrong msgType) \n";
      // LogContext(logContext).logErr(
      //    "Received invalid response type: " + rrArgs->outRespMsg->getMsgTypeStr() + "; "
      //    "expected: " + netMessageTypeToStr(rrArgs->respMsgType) + ". "
      //    "Disconnecting: " + node.getNodeIDWithTypeStr() + " @ " +
      //    sock->getPeername() );

      retVal = FhgfsOpsErr_COMMUNICATION;
      // goto err_cleanup;
   }

   retVal = FhgfsOpsErr_SUCCESS;
   // err_cleanup:
   // 释放套接字
   NodeConnPool_releaseStreamSocket(sock);

   return retVal;
   /*
         // got correct response

         // connPool->releaseStreamSocket(sock);

         return FhgfsOpsErr_SUCCESS;
      }
      catch (const std::bad_alloc& e)
      {
         LOG(COMMUNICATION, ERR, "Memory allocation for send buffer failed.");
         retVal = FhgfsOpsErr_OUTOFMEM;
      }
      catch(SocketConnectException& e)
      {
         if ( !(rrArgs->logFlags & REQUESTRESPONSEARGS_LOGFLAG_CONNESTABLISHFAILED) )
         {
            LOG(GENERAL, WARNING, "Unable to connect, is the node offline?", ("node", node.getNodeIDWithTypeStr()),
                  ("Message type", rrArgs->requestMsg->getMsgTypeStr()));
         }

         retVal = FhgfsOpsErr_COMMUNICATION;
      }
      catch(SocketException& e)
      {
         LogContext(logContext).logErr("Communication error: " + std::string(e.what() ) + "; " +
            "Peer: " + node.getNodeIDWithTypeStr() + ". "
            "(Message type: " + rrArgs->requestMsg->getMsgTypeStr() + ")");

         retVal = FhgfsOpsErr_COMMUNICATION;
      }


   err_cleanup:

      // clean up...

      if(sock)
         connPool->invalidateStreamSocket(sock);
   */
}

// bool MessagingTk::requestResponse(RequestResponseArgs* rrArgs)
bool requestResponse(RequestResponseArgs *rrArgs)
{
   FhgfsOpsErr commRes = requestResponseComm(rrArgs);
   // One retry in case the connection was already broken when we got it (e.g. peer daemon restart).
   if (commRes == FhgfsOpsErr_COMMUNICATION)
   {
      // LOG(COMMUNICATION, WARNING, "Retrying communication.",
      //      ("peer",  rrArgs->node->getNodeIDWithTypeStr()),
      //      ("message type", rrArgs->requestMsg->getMsgTypeStr()));
      LOG(DEBUG) << "[requestResponse(rrArgs)] error, Retrying communication. \n ";
      commRes = requestResponseComm(rrArgs);
   }

   return commRes == FhgfsOpsErr_SUCCESS;
}

/**
 * Sends a message and receives the response.
 *
 * @param outRespMsg response message
 * @return true if communication succeeded and expected response was received
 */
// std::unique_ptr<NetMessage> MessagingTk::requestResponse(Node_class& node, NetMessage& requestMsg,unsigned respMsgType)
std::unique_ptr<NetMessage_class> requestResponse(Node_class &node, NetMessage_class &requestMsg, unsigned respMsgType)
{
   RequestResponseArgs rrArgs(&node, &requestMsg, respMsgType);

   if (requestResponse(&rrArgs))
      return std::move(rrArgs.outRespMsg);

   return nullptr;
}

// inline FhgfsOpsErr MetadataTk::findOwnerStep(Node& node,NetMessage* requestMsg, EntryInfoWithDepth* outEntryInfoWDepth)
inline FhgfsOpsErr findOwnerStep(Node_class &node, NetMessage_class *requestMsg, EntryInfoWithDepth *outEntryInfoWDepth)
{
   FindOwnerRespMsg *findResp;
   FhgfsOpsErr findRes;

   // const auto respMsg = MessagingTk::requestResponse(node, *requestMsg, NETMSGTYPE_FindOwnerResp);
   const auto respMsg = requestResponse(node, *requestMsg, NETMSGTYPE_FindOwnerResp);

   if (!respMsg)
   { // communication error
      return FhgfsOpsErr_INTERNAL;
   }

   // handle result
   findResp = (FindOwnerRespMsg *)respMsg.get();
   findRes = (FhgfsOpsErr)findResp->getResult();

   if (findRes == FhgfsOpsErr_SUCCESS)
   {
      LOG(DEBUG) << " 获取成功 \n";
      *outEntryInfoWDepth = findResp->getEntryInfo();
   }

   return findRes;
}

// 把 getentryid.CPP 的关键函数移动到这里
//  FhgfsOpsErr Path_resolution(char* inputPathStr,DirEntryType inputEntryType,NumNodeID& outOwnerNodeID, std::string& outParentEntryID,std::string& outEntryID, std::string& outFileName,DirEntryType& outEntryType, int& outFeatureFlags)

FhgfsOpsErr Path_resolution(char *inputPathStr, DirEntryType inputEntryType, NumNodeID &outOwnerNodeID, char *outParentEntryID, char *outEntryID, char *outFileName, DirEntryType &outEntryType, int &outFeatureFlags)
// FhgfsOpsErr Path_resolution()
{
   // EntryInfo* outEntryInfo=nullptr;

   // 参数1  传入参数 PS： 参数3 初始化的时候有 searchPath->back(),这个操作

   // 为什么这些查询结果的 entry type =0 表示无效DirEntryType_INVALID  问题出在 direntrytype 和 unsigned int 的强制类型转换，本质上是我对 FindOwnerRespMsg 的反序列化解析的逻辑出了问题
   //  std::string cfgPathStr="/test/dir1/1.txt";//@@ 我给它初始化吧
   //  std::string cfgPathStr="test_dir/dir1/test1.txt";//@@ 我给它初始化吧
   // 路径可以获取正确的ID
   //  std::string cfgPathStr="test_dir";//@@ 我给它初始化吧
   //  std::string cfgPathStr="test_dir/dir1";//@@ 我给它初始化吧
   //  std::string cfgPathStr="test_dir/1.txt";//@@ 我给它初始化吧
   //  std::string cfgPathStr="test_dir/dir1";//@@ 我给它初始化吧
   //  std::string cfgPathStr="test_dir/dir1/1.txt";//@@ 我给它初始化吧
   //  std::string cfgPathStr="test_dir/dir1/dir11";//@@ 我给它初始化吧
   //  std::string cfgPathStr="test_dir/dir1/dir11/test11.txt";//@@ 我给它初始化吧
   //  std::string cfgPathStr="test_dir/dir1/dir11/test11.txt";//@@ 我给它初始化吧 ,为什么这个的Feature Flags: 1

   std::string cfgPathStr = inputPathStr; //@@ 我给它初始化吧 ,为什么这个的Feature Flags: 1

   // 为 1 的含义是 STATFLAG_HINT_INLINE宏定义了一个标志， 用于提供关于 inode 是否内联到 dentry 中的提示，内联 inode 可以优化文件系统操作

   // 错误的路径
   //  std::string cfgPathStr="test_dir/dir1/test1.txt";//@@ 我给它初始化吧

   Path searchPath(cfgPathStr); // Path* searchPath

   LOG(DEBUG) << "---------begin\n";

   // 参数2 传入参数

   size_t searchDepth = searchPath.size(); // 0-based incl. root
   LOG(DEBUG) << " searchDepth = " << searchDepth << "\n";
   if (!searchDepth) // 路径深度
   {
      LOG(DEBUG) << "// looking for the root node (ignore referenceParent)\n"; //@@ MetadataTk::referenceOwner 这个函数实现里有对应的处理逻辑，我先省略了
   }

   // 参数 currentNode

   // reference root node
   //    auto currentNode = referenceRoot(*nodes, *metaRoot, *metaBuddyGroupMapper);//
   auto currentNode = referenceRoot(); // 返回的是元数据节点

   if (!currentNode)
   {
      //   LogContext(logContextStr).logErr("Unable to proceed without a working root metadata server");
      LOG(DEBUG) << "currentNode==null , Unable to proceed without a working root metadata server \n";
      return FhgfsOpsErr_UNKNOWNNODE;
   }

   // 参数3
   LOG(DEBUG) << "currentNode->getNumID() = " << currentNode->getNumID().value << "\n";
   EntryInfo_class lastEntryInfo(currentNode->getNumID(), "", META_ROOTDIR_ID_STR,
                                 // searchPath.back(), DirEntryType_DIRECTORY, 0);//DirEntryType_REGULARFILE
                                 searchPath.back(), inputEntryType, 0); // DirEntryType_REGULARFILE

   LOG(DEBUG) << "searchPath.back() =" << searchPath.back() << "\n";
   //  EntryInfo_class(const NumNodeID ownerNodeID, const std::string& parentEntryID,
   //    const std::string& entryID, const std::string& fileName, const DirEntryType entryType,
   //    const int featureFlags) :

   // 参数4
   unsigned lastEntryDepth = 0;

   FindOwnerMsg requestMsg(&searchPath, searchDepth, &lastEntryInfo, lastEntryDepth);

   EntryInfoWithDepth entryInfoWDepth;

   FhgfsOpsErr findRes;

   findRes = findOwnerStep(*currentNode, (NetMessage_class *)&requestMsg, &entryInfoWDepth);
   if (entryInfoWDepth.getEntryDepth() == searchDepth)
   { // successful end of search
     // outEntryInfo->set(&entryInfoWDepth);
      LOG(DEBUG) << "successful end of search \n";
   }
   else
   {
      LOG(DEBUG) << "failed entryInfoWDepth.getEntryDepth() != searchDepth \n";
   }
   // 设置 entryInfo 的成员变量

   NumNodeID ownerNodeID;
   std::string parentEntryID, entryID, fileName;
   DirEntryType entryType;
   int featureFlags;
   LOG(DEBUG) << "待查询的路径信息 = " << cfgPathStr << "\n";
   // NumNodeID& outOwnerNodeID, std::string& outParentEntryID,std::string& outEntryID, std::string& outFileName,DirEntryType& outEntryType, int& outFeatureFlags)
   entryInfoWDepth.getEntryInfo(outOwnerNodeID, outParentEntryID, outEntryID, outFileName, outEntryType, outFeatureFlags);

   return FhgfsOpsErr_SUCCESS;
}
void PrintEntryInfo_string(NumNodeID &outOwnerNodeID, std::string &outParentEntryID,
                           std::string &outEntryID, std::string &outFileName,
                           DirEntryType &outEntryType, int &outFeatureFlags)
{
   LOG(DEBUG) << "--------- print out Entryinfo ----------- \n";
   LOG(DEBUG) << "Owner Node ID: " << outOwnerNodeID.value << std::endl;
   LOG(DEBUG) << "Parent Entry ID: " << outParentEntryID << std::endl;
   LOG(DEBUG) << "Entry ID: " << outEntryID << std::endl;
   LOG(DEBUG) << "File Name: " << outFileName << std::endl;
   //   LOG(DEBUG) << "Entry Type: " << static_cast<int>(outEntryType) << std::endl;
   LOG(DEBUG) << "Entry Type: " << (outEntryType) << std::endl;

   LOG(DEBUG) << "Feature Flags: " << outFeatureFlags << std::endl;
   LOG(DEBUG) << "--------- ----------- ----------- \n";
}

void PrintEntryInfo_char(NumNodeID &outOwnerNodeID, char *outParentEntryID,
                         char *outEntryID, char *outFileName,
                         DirEntryType &outEntryType, int &outFeatureFlags)
{
   LOG(INFO) << "--------- print out Entryinfo ----------- \n";
   LOG(INFO) << "Owner Node ID: " << outOwnerNodeID.value << std::endl;
   LOG(INFO) << "Parent Entry ID: " << outParentEntryID << std::endl;
   LOG(INFO) << "Entry ID: " << outEntryID << std::endl;
   LOG(INFO) << "File Name: " << outFileName << std::endl;
   //   LOG(INFO) << "Entry Type: " << static_cast<int>(outEntryType) << std::endl;
   LOG(INFO) << "Entry Type: " << (outEntryType) << std::endl;

   LOG(INFO) << "Feature Flags: " << outFeatureFlags << std::endl;
   LOG(INFO) << "--------- ----------- ----------- \n";
}

void test(char **&list, int cnt)
{
   list = (char **)malloc(cnt * sizeof(char *));
   if (list == nullptr)
   {
      LOG(DEBUG) << "分配内存失败 \n";
      return;
   }
   for (int i = 0; i < cnt; i++)
   {
      list[i] = (char *)malloc(sizeof(char) * 25);
      if (list[i] == nullptr)
      {
         LOG(DEBUG) << "分配内存失败2 \n";
         for (int j = 0; j < i; j++)
         {
            free(list[j]);
         }
         free(list);
         return;
      }
   }
   LOG(DEBUG) << "分配成功 \n";
   LOG(DEBUG) << " list = " << list << "\n";
   return;
}
void PrintResultOfReaddir(int elemcnt, char **namelist, char **idlist, int *typelist)
{
   LOG(INFO) << "--- start PrintResultOfReaddir ------\n";

   LOG(INFO) << "----------- print namelist -----------\n";
   for (int i = 0; i < elemcnt; i++)
   {
      LOG(INFO) << "NO." << i << "  :  " << namelist[i] << "\n";
   }
   LOG(INFO) << "----------- print idlist -----------\n";
   for (int i = 0; i < elemcnt; i++)
   {
      LOG(INFO) << "NO." << i << "  :  " << idlist[i] << "\n";
   }
   LOG(INFO) << "----------- print typelist -----------\n";
   for (int i = 0; i < elemcnt; i++)
   {
      LOG(INFO) << "NO." << i << "  :  " << typelist[i] << "\n";
   }
   LOG(INFO) << "--- end of PrintResultOfReaddir ------\n";
}

// XYP_MODIFY 0304
char FileHandleID_1[50];
char FileHandleID_2[50];
char FileHandleID_3[50];
char FileHandleID_4[50];
#define FILE_NUM 1024
#define MAX_FILE_NUM 10450
#define MAX_REQUEST_LEN 256
#define MAX_FILE_PATH_LEN 32 // 凑成 8 的倍数，预防内存对齐问题
struct file_info
{
   char file_path[MAX_FILE_PATH_LEN];
   uint64_t file_begin;
   uint64_t file_size;
};

/**
 * @brief Generate Beegfs Reads files request from a directory into a receive buffer.
 *
 * This function reads all regular files (excluding those ending with ".gz") from the specified directory
 * and stores their contents into a receive buffer. It also populates an array of file_info structures
 * with information about each file read.
 * [in]
 * @param inputsearchPath The directory Path to read files from.
 * @param vaddr Virtual Address of Client local buffer for BeeGFS-Storage to RDMA_Write File contents to.
 * @param rkey  Remote Key for BeeGFS-Storage to RDMA_Write File contents to Client local buffer.
 * [out]
 * @param file_num A pointer to a uint32_t to store the number of files read.
 * @param file_infos An array of file_info structures to store information about each file read.
 * @param msg_buf The destination buffer to store the file requests.
 * @param msg_len An array of the length info of each file requests , that is, the start offset of each file request in msg_buf
 *
 */

struct timespec time_Pathresolution, time_Readdirlist, time_Openread, singleopen_start, singleopen_end; // 变量的实际定义
double Total_Open = 0;
void read_dir(char *inputsearchPath, uint64_t vaddr, uint32_t rkey,
              uint32_t *file_num, struct file_info *file_info_arr, char *msg_buf, unsigned int *msg_len)
{

   // 1.路径解析，获取目标目录、文件的 EntryInfo
   LOG_printf(DEBUG, "----------------- 1.路径解析，获取目标目录、文件的 EntryInfo --------------------------\n");
   // char inputsearchPath[50]="FashionMNIST";
   // DirEntryType searchEntryType=DirEntryType_REGULARFILE;// or DirEntryType_DIRECTORY
   // char searchPath[50]="small_image";
   char searchPath[50];
   strcpy(searchPath, inputsearchPath);
   LOG(DEBUG) << "searchPath = " << searchPath << "\n";
   DirEntryType searchEntryType = DirEntryType_DIRECTORY; // DirEntryType_REGULARFILE or DirEntryType_DIRECTORY

   NumNodeID outOwnerNodeID;
   char *ParentEntryID = (char *)malloc(50 * sizeof(char));
   char *currentDirEntryID = (char *)malloc(50 * sizeof(char));
   char *currentDirName = (char *)malloc(50 * sizeof(char));
   DirEntryType currentDirType;
   int outFeatureFlags;

   FhgfsOpsErr res = Path_resolution(searchPath, searchEntryType, outOwnerNodeID, ParentEntryID, currentDirEntryID, currentDirName, currentDirType, outFeatureFlags);
   // 标识 与 BGFS-meta Path_resolution 的时间
   clock_gettime(CLOCK_MONOTONIC, &time_Pathresolution);

   // PrintEntryInfo_char(outOwnerNodeID, ParentEntryID, currentDirEntryID, currentDirName, currentDirType, outFeatureFlags);

   LOG_printf(DEBUG, "----------------- 2. readdir  --------------------------\n");

   Socket *sock = nullptr;

   char **namelist = nullptr;
   char **idlist = nullptr;
   int *typelist = nullptr;
   int DirElemCnt = 0;
   //  test(namelist,DirElemCnt);

   readdir(sock, ParentEntryID, currentDirEntryID, currentDirName, currentDirType, namelist, idlist, typelist, DirElemCnt, file_num); // 这里可以获取文件的数量

   // 标识 与 BGFS-meta Readdirlist 的时间
   clock_gettime(CLOCK_MONOTONIC, &time_Readdirlist);
   // PrintResultOfReaddir(DirElemCnt,namelist,idlist,typelist);

   LOG_printf(DEBUG, "------------------ 3. 依次open /read 对应的文件内容 -------------------\n");
   // void open(Socket* sock,char* parentEntryID,char* entryID,char* fileName,DirEntryType entryType,char* OutFileHandleID);

   LOG(DEBUG) << "sock = " << sock << "\n";
   
   // DEBUG 认为设置下 file_num
   int up_file_num = FILE_NUM;
   *file_num = up_file_num;
   LOG(INFO) << "file_num = " << *file_num << "\n";
   uint32_t file_idx = 0; // 统计文件数量

   uint64_t file_size[MAX_FILE_NUM]; // 上帝视角的数据设置
   for (int i = 0; i < *file_num; i++)
   {
      file_size[i] =512;// 32 * 1024; // 57*1024;
   }
   // std::fill(file_size, file_size + *file_num, 32 * 1024);

   // file_size[0]=1024;//7840016;//7840016;//7840016;//8*1024*1024;//4096;//7840016;
   // file_size[1]=1024;//60008;//60008;//60008;//60008;
   // file_size[2]=1024;//47040016;
   // file_size[3]=1024;//10008;

   // file_size[0]=7840016;//7840016;//7840016;//8*1024*1024;//4096;//7840016;
   // file_size[1]=60008;//60008;//60008;//60008;
   // file_size[2]=47040016;
   // file_size[3]=10008;

   uint64_t vaddr_offset[MAX_FILE_NUM];
   uint64_t info_start_off =0;// sizeof(uint32_t) + (*file_num) * (sizeof(struct file_info));
   LOG_printf(DEBUG, "file_num %u , info_start_off : %lu \n", *file_num, info_start_off);
   vaddr_offset[file_idx] = info_start_off;
   Socket *RDMA_sock = nullptr;
   // 建立RDMA 连接
   if (RDMA_sock == NULL)
   {

      LOG_printf(DEBUG, "----- 1. 创建套接字 -----\n");
      struct in_addr srcAddr;
      NicAddress *nicAddr = (NicAddress *)malloc(sizeof(NicAddress));
      NicAddressStats *srcRdma = (NicAddressStats *)malloc(sizeof(NicAddressStats));

      const char *LocalRdmaIP = "192.168.5.209"; // 本地dell02的RDMA IP,   网口名为 enp130s0np0
                                                 //@@本地 dell01 的RDMA IP : 192.168.0.203 ,   网口名为 enp152s0np0
                                                 //  164Client ens4f0np0  192.168.3.212
                                                 // 164 client ens5f0np0 192.168.5.209

      if (inet_pton(AF_INET, LocalRdmaIP, &srcAddr) <= 0)
      { // 将点分十进制的IP地址转换为用于网络传输的数值格式。
         perror("Invalid IP address");
         return;
      }
      LOG_printf(DEBUG, "查看本主机IP情况 LocalIP %s and tranfer to %u\n", LocalRdmaIP, srcAddr.s_addr);
      nicAddr->ipAddr = srcAddr;
      nicAddr->nicType = NICADDRTYPE_RDMA;
      strncpy(nicAddr->name, "ens5f0np0", IFNAMSIZ); //@@本地 RDMA 端口名 dell02 : enp130s0np0
                                                     //@@本地 dell01 的RDMA IP : 192.168.0.203 ,   网口名为 enp152s0np0
                                                     // 164Client ens4f0np0  192.168.3.212
                                                 // 164 client ens5f0np0 192.168.5.209


      NicAddressStats_init(srcRdma, nicAddr);

      RDMA_sock = (Socket *)RDMASocket_construct(srcAddr, srcRdma); // 构造一个RDMA socket
      if (RDMA_sock == NULL)
         LOG_printf(DEBUG, "@@ Error ----------- 3 ---------------\n");
      if (!RDMA_sock)
      { // no conn available => error or didn't want to wait
         LOG_printf(DEBUG, "----------------获取RDMA socket失败!----------------------\n");
         {

            if (RDMA_sock)
            {
               NodeConnPool_releaseStreamSocket(RDMA_sock); // 这里的释放类型要注意
            }
         }
         return;
      }
      else
      {
         LOG_printf(DEBUG, "----------------获取RDMA socket成功!----------------------\n");
      }

      // 3. 尝试通过IP连接
      if (RDMA_sock == NULL)
         LOG_printf(DEBUG, "@@ Error ----------- 4 ---------------\n");
      LOG_printf(DEBUG, "------------------- 3. 尝试通过IP连接-------------------\n");

      struct in_addr destAddr;
      const char *RemoteRdmaIP = "192.168.5.210"; //@@存储服务器xfusion3的RDMA IP 192.168.0.218 ,网口名 ens2np0
                                                  // 162Server ens4f1np1 192.168.3.216 mlx5_0
                                                  // 162server ens4f0np0 192.168.5.210

      if (inet_pton(AF_INET, RemoteRdmaIP, &destAddr) <= 0)
      {
         perror("Invalid remote IP address");
         return;
      }
      LOG_printf(DEBUG, "查看远端主机IP情况 RemoteIP %s and tranfer to %u\n", RemoteRdmaIP, destAddr.s_addr);

      unsigned short port = 8005; // 存储服务器的port 8003 元数据服务器的端口是 8005
      LOG_printf(DEBUG, "----------------开始根据IP与远端建立RDMA连接-----------------------\n");
      // bool connectRes = info->socket->ops->connectByIP(info->socket, destAddr, port);
      bool connectRes = RDMA_sock->ops->connectByIP(RDMA_sock, destAddr, port);

      if (connectRes)
      {
         LOG_printf(DEBUG, "尝试通过IP连接 连接成功 \n");
      }
      else
      {
         LOG_printf(DEBUG, "尝试通过IP连接 连接失败 \n");
      }

      if (RDMA_sock == NULL)
         LOG_printf(DEBUG, "@@ Error ----------- 5 ---------------\n");
      // 这是 服务端的认证配置
      bool isflag = __NodeConnPool_applySocketOptionsConnected(RDMA_sock);
      if (isflag == true)
      {
         LOG(DEBUG) << "__NodeConnPool_applySocketOptionsConnected 成功 ！！\n";
      }
      else
      {
         LOG(DEBUG) << "__NodeConnPool_applySocketOptionsConnected 失败 ！！\n";
      }
      if (RDMA_sock == NULL)
         LOG_printf(DEBUG, "@@ Error ----------- 6 ---------------\n");
   }

   for (int i = 0; i < DirElemCnt; i++)
   {
      if (typelist[i] == DirEntryType_REGULARFILE)
      {

         char *FileHandleID = (char *)malloc(50 * sizeof(char));
         clock_gettime(CLOCK_MONOTONIC, &singleopen_start);

         // open(sock,currentDirEntryID,idlist[i],namelist[i], DirEntryType_REGULARFILE,FileHandleID);//
         // 替换成RDMA 通信模式的 open
         if (RDMA_sock == NULL)
            LOG(DEBUG) << "@@ error " << file_idx << "th file \n";
         open_RDMA(RDMA_sock, currentDirEntryID, idlist[i], namelist[i], DirEntryType_REGULARFILE, FileHandleID); //
         LOG_printf(DEBUG, "@@ 后续处理 Error sock %p \n", RDMA_sock);

         if (RDMA_sock == NULL)
            LOG(DEBUG) << "@@ error " << file_idx << "th file \n";
         // 标识 与 BGFS-meta open 加 read request 生成的总时间
         clock_gettime(CLOCK_MONOTONIC, &singleopen_end);
         Total_Open += (singleopen_end.tv_sec - singleopen_start.tv_sec) + (singleopen_end.tv_nsec - singleopen_start.tv_nsec) / 1e9;

         LOG(DEBUG) << "FileName: " << namelist[i] << "\t, FileHandleID: " << FileHandleID << "\n";

         unsigned int origParentUID = 0; // 上帝视角的数据设置 image_32KB_500 image_32KB
         // unsigned int origParentUID=1000;//上帝视角的数据设置 test_dir(500个512B文件) small_image (12个文件)

         char *origParentEntryID = (char *)malloc(50 * sizeof(char));
         strcpy(origParentEntryID, currentDirEntryID);
         LOG(DEBUG) << "origParentEntryID = " << origParentEntryID << "\n";
         LOG(DEBUG) << "--------------- open 成功 ---------------------\n";
         LOG(DEBUG) << "--------------- 开始 read  ---------------------\n";
         // size_t toberead=8481;
         // struct iovec* requestedheader=(struct iovec*)malloc(sizeof(struct iovec));
         // FhgfsOpsRemoting_readfileVec(FileHandleID,origParentUID,origParentEntryID,toberead,requestedheader);

         size_t toberead = file_size[file_idx];
         unsigned int cur_msg_len = 0;
         int begin_offset = 0;        // 默认参数，读取begin_offset =0
         size_t vaddr_len = toberead; // unsigned int型变量  2^32-1=3.99*1024*1024*1024 ~ 4GB
         // printf("%u th file vaddr_offset = %lu \n",file_idx,vaddr_offset[file_idx]);
         generate_read_request(FileHandleID, origParentUID, origParentEntryID, msg_buf + MAX_REQUEST_LEN * file_idx, &cur_msg_len, vaddr + vaddr_offset[file_idx], rkey, vaddr_len, toberead, begin_offset);
         msg_len[file_idx] = cur_msg_len;
         vaddr_offset[file_idx + 1] = vaddr_offset[file_idx] + file_size[file_idx];

         // 填充file_info
         LOG_printf(DEBUG, "namelist[%u] %s, sizeof namelist[%u] %lu , strlen(namelist[i]): %u \n", file_idx, namelist[file_idx], file_idx, sizeof(namelist[i]), strlen(namelist[i]));
         // strcpy(file_info_arr[file_idx].file_path,namelist[i]);
         memset(file_info_arr[file_idx].file_path, 0x00, MAX_FILE_PATH_LEN);
         memcpy(file_info_arr[file_idx].file_path, namelist[i], strlen(namelist[i]));
         file_info_arr[file_idx].file_size = file_size[file_idx];
         file_info_arr[file_idx].file_begin = vaddr_offset[file_idx];
         LOG_printf(DEBUG, "file_info[%u].file_path: %s \n", file_idx, file_info_arr[file_idx].file_path);
         LOG_printf(DEBUG, "file_info[%u].file_size: %lu \n", file_idx, file_info_arr[file_idx].file_size);
         LOG_printf(DEBUG, "file_info[%u].file_begin: %lu \n", file_idx, file_info_arr[file_idx].file_begin);

         // 打开文件，准备写入
         // FILE* file = fopen("output.txt", "wb"); // "w" 模式表示写入，并在文件不存在时创建文件
         // if (file == NULL) {
         //    perror("Error opening file");
         //    return ;
         // }

         // // 写入数据到文件
         // fwrite(requestedheader->iov_base, sizeof(char), requestedheader->iov_len, file);

         // // 关闭文件
         // fclose(file);
         file_idx++;
         LOG_printf(DEBUG, "------------- Finish generate %uth File Request ------------\n", file_idx);
         free(FileHandleID);
         free(origParentEntryID);

         if (file_idx >= up_file_num)
            break;
      }
      // else if(typelist[i]==DirEntryType_DIRECTORY)//还需要继续 readdir
      // {

      // }
   }
   // *file_num=file_idx;
   // 标识 与 BGFS-meta open 加 read request 生成的总时间
   clock_gettime(CLOCK_MONOTONIC, &time_Openread);

   // 1.释放内存 路径解析阶段的
   free(ParentEntryID);
   free(currentDirEntryID);
   free(currentDirName);

   // 2. 释放内存 namelist idlist typelist readdir阶段的

   // 释放内存 namelist
   if (namelist != nullptr)
   {
      for (int i = 0; i < DirElemCnt; i++)
      {
         free(namelist[i]); // 释放每个字符串的内存
      }
      free(namelist); // 释放指针数组
   }

   // 释放内存 idlist
   if (idlist != nullptr)
   {
      for (int i = 0; i < DirElemCnt; i++)
      {
         free(idlist[i]); // 释放每个字符串的内存
      }
      free(idlist); // 释放指针数组
   }

   // 释放内存 typelist

   free(typelist); // 释放指针数组
   // 3.释放套接字
   NodeConnPool_releaseStreamSocket(sock);
   NodeConnPool_releaseStreamSocket(RDMA_sock);

   return;
}
/*
old version 250306 past
void read_dir()
{
   //1.路径解析，获取目标目录、文件的 EntryInfo
   LOG_printf(INFO,"----------------- 1.路径解析，获取目标目录、文件的 EntryInfo --------------------------\n");
   // char inputsearchPath[50]="FashionMNIST";
   // DirEntryType searchEntryType=DirEntryType_REGULARFILE;// or DirEntryType_DIRECTORY
   // char searchPath[50]="small_image";
   char searchPath[50];
   strcpy(searchPath,inputsearchPath);
   LOG(DEBUG)<<"searchPath = "<<searchPath<<"\n";
   DirEntryType searchEntryType=DirEntryType_DIRECTORY;// DirEntryType_REGULARFILE or DirEntryType_DIRECTORY

   NumNodeID outOwnerNodeID;
   char* ParentEntryID=(char*)malloc(50*sizeof(char));
   char* currentDirEntryID=(char*)malloc(50*sizeof(char));
   char* currentDirName=(char*)malloc(50*sizeof(char));
   DirEntryType currentDirType;
   int outFeatureFlags;

   FhgfsOpsErr res = Path_resolution(searchPath,searchEntryType,outOwnerNodeID, ParentEntryID, currentDirEntryID, currentDirName, currentDirType, outFeatureFlags);
   PrintEntryInfo_char(outOwnerNodeID, ParentEntryID, currentDirEntryID, currentDirName, currentDirType, outFeatureFlags);

   LOG_printf(INFO,"----------------- 2. readdir  --------------------------\n");

   Socket* sock=nullptr;

    char** namelist=nullptr;
    char** idlist=nullptr;
    int* typelist=nullptr;
    int DirElemCnt=0;
   //  test(namelist,DirElemCnt);


    readdir(sock,ParentEntryID,currentDirEntryID,currentDirName,currentDirType,namelist,idlist,typelist,DirElemCnt);

      PrintResultOfReaddir(DirElemCnt,namelist,idlist,typelist);

   LOG_printf(INFO,"------------------ 3. 依次open /read 对应的文件内容 -------------------\n");
// void open(Socket* sock,char* parentEntryID,char* entryID,char* fileName,DirEntryType entryType,char* OutFileHandleID);

LOG(DEBUG)<<"sock = "<<sock<<"\n";
   // for(int i=0;i<DirElemCnt;i++)
   int file_cnt=0;//统计文件数量
   // uint64_t file_size[MAX_FILE_NUM];
   // file_size[0]=7840016;
   // file_size[1]=60008;
   // file_size[2]=47040016;
   // file_size[3]=10008;

   for(int i=0;i<DirElemCnt;i++)
   {
      if(typelist[i]==DirEntryType_REGULARFILE)
      {
         char* FileHandleID=(char*)malloc(50*sizeof(char));
         open(sock,currentDirEntryID,idlist[i],namelist[i], DirEntryType_REGULARFILE,FileHandleID);
         LOG(DEBUG)<<"FileName: "<<namelist[i]<<"\t, FileHandleID: "<<FileHandleID<<"\n";
         unsigned int origParentUID=29999;
         char * origParentEntryID=(char*)malloc(50*sizeof(char));
         strcpy(origParentEntryID,currentDirEntryID);
         LOG(DEBUG)<<"origParentEntryID = "<<origParentEntryID<<"\n";
         LOG(DEBUG)<<"--------------- open 成功 ---------------------\n";
         LOG(DEBUG)<<"--------------- 开始 read  ---------------------\n";
         // size_t toberead=8481;
         // struct iovec* requestedheader=(struct iovec*)malloc(sizeof(struct iovec));
         // FhgfsOpsRemoting_readfileVec(FileHandleID,origParentUID,origParentEntryID,toberead,requestedheader);

         size_t toberead=10;
         file_cnt++;
         if(file_cnt == 1)
            {
               memcpy(FileHandleID_1,FileHandleID,strlen(FileHandleID));
               // generate_read_request(FileHandleID,origParentUID,origParentEntryID,
               //    msg_buf,msg_len,vaddr, rkey,vaddr_len,toberead,begin_offset);
               //    break;
            }
            else if( file_cnt ==2)
               memcpy(FileHandleID_2,FileHandleID,strlen(FileHandleID));
            else if(file_cnt ==3)
               memcpy(FileHandleID_3,FileHandleID,strlen(FileHandleID));
               else if(file_cnt ==4)
               memcpy(FileHandleID_4,FileHandleID,strlen(FileHandleID));



         // 打开文件，准备写入
         // FILE* file = fopen("output.txt", "wb"); // "w" 模式表示写入，并在文件不存在时创建文件
         // if (file == NULL) {
         //    perror("Error opening file");
         //    return ;
         // }

         // // 写入数据到文件
         // fwrite(requestedheader->iov_base, sizeof(char), requestedheader->iov_len, file);

         // // 关闭文件
         // fclose(file);

         // ssize_t FhgfsOpsRemoting_readfileVec(char * filehandleID,unsigned int origParentUID,char * origParentEntryID)

         LOG(DEBUG)<<"出来了 \n";
         free(FileHandleID);
         free(origParentEntryID);
         // break;
      }
      else if(typelist[i]==DirEntryType_DIRECTORY)//还需要继续 readdir
      {

      }
   }
   // *file_num=file_cnt;







   //1.释放内存 路径解析阶段的
      free(ParentEntryID);
      free(currentDirEntryID);
      free(currentDirName);

   //2. 释放内存 namelist idlist typelist readdir阶段的

      //释放内存 namelist
      if(namelist!=nullptr)
         {
               for (int i = 0; i < DirElemCnt; i++) {
                  free(namelist[i]); // 释放每个字符串的内存
               }
               free(namelist); // 释放指针数组
         }


      //释放内存 idlist
      if(idlist!=nullptr)
        {
            for (int i = 0; i < DirElemCnt; i++) {
                     free(idlist[i]); // 释放每个字符串的内存
               }
            free(idlist); // 释放指针数组
        }

      //释放内存 typelist

         free(typelist); // 释放指针数组
   //3.释放套接字
   NodeConnPool_releaseStreamSocket(sock);
        return ;
}
*/
#endif