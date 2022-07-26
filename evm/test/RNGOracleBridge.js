const { expect } = require("chai");
const { ethers } = require("hardhat");
const ONE_TLOS = ethers.utils.parseEther("1.0");
const HALF_TLOS = ethers.utils.parseEther("0.5");
const MAX_REQUESTS = 10;
const MAX_NUMBER_COUNT_PER_REQUEST = 10;
const GAS_PRICE = 499;

describe("RNGOracleBridge Contract", function () {
    let bridge, owner, oracle, oracle2, user, consumer, gasOracle;
    beforeEach(async () => {
        [owner, oracle, oracle2, user] = await ethers.getSigners();
        let Consumer = await ethers.getContractFactory("RNGOracleConsumer");
        let Bridge = await ethers.getContractFactory("RNGOracleBridge");
        let GasOracle = await ethers.getContractFactory("GasOracleBridge");
        gasOracle = await GasOracle.deploy(owner.address, GAS_PRICE)
        bridge = await Bridge.deploy(ONE_TLOS, MAX_REQUESTS, MAX_NUMBER_COUNT_PER_REQUEST, oracle.address, gasOracle.address);
        consumer = await Consumer.deploy(bridge.address);
    })
    describe(":: Setters", function () {
        it("Should allow owner to set the fee" , async function () {
            await expect(bridge.setFee(HALF_TLOS)).to.not.be.reverted;
            expect(await bridge.fee()).to.equal(HALF_TLOS);
        });
        it("Should allow owner to set the Oracle EVM address" , async function () {
            await expect(bridge.setOracleEVMAddress("0x6c33bdd2622e59fd10b411ff8d8d8d4dc5caf6c1")).to.not.be.reverted;
            let cc = await bridge.oracle_evm_address();
            expect(cc.toLowerCase()).to.equal("0x6c33bdd2622e59fd10b411ff8d8d8d4dc5caf6c1");
        });
        it("Should allow owner to set the max number count per request" , async function () {
            await expect(bridge.setMaxNumberCount(MAX_NUMBER_COUNT_PER_REQUEST * 2)).to.not.be.reverted;
            expect(await bridge.max_number_count()).to.equal(MAX_NUMBER_COUNT_PER_REQUEST * 2);
        });
        it("Should allow owner to set the max requests" , async function () {
            await expect(bridge.setMaxRequests(MAX_REQUESTS / 2)).to.not.be.reverted;
            expect(await bridge.max_requests()).to.equal(MAX_REQUESTS / 2);
        });
        it("Shouldn't allow addresses other than owner to set the Oracle's EVM address" , async function () {
            await expect(bridge.connect(user).setOracleEVMAddress("0x6c33bdd2622e59fd10b411ff8d8d8d4dc5caf6c1")).to.be.reverted;
        });
        it("Shouldn't allow addresses other than owner to set the max number count per request" , async function () {
            await expect(bridge.connect(user).setMaxNumberCount(MAX_NUMBER_COUNT_PER_REQUEST * 2)).to.be.reverted;
        });
        it("Shouldn't allow addresses other than owner to set the fee" , async function () {
            await expect(bridge.connect(user).setFee(HALF_TLOS)).to.be.reverted;
        });
        it("Shouldn't allow addresses other than owner to set the max requests" , async function () {
            await expect(bridge.connect(user).setMaxRequests(MAX_REQUESTS)).to.be.reverted;
        });
    });
    describe(":: Getters", function () {
        it("Should be able to return the fee" , async function () {
            expect(await bridge.fee()).to.equal(ONE_TLOS);
        });
        it("Should be able to return the max requests" , async function () {
            expect(await bridge.max_requests()).to.equal(MAX_REQUESTS);
        });
        it("Should be able to return the max number count" , async function () {
            expect(await bridge.max_number_count()).to.equal(MAX_NUMBER_COUNT_PER_REQUEST);
        });
    });
    describe(":: Request Price", function () {
        it("Should be able to calculate the request price correctly" , async function () {
            await expect(bridge.calculateRequestPrice(41000)).to.not.be.reverted;
            expect(await bridge.calculateRequestPrice(41000)).to.be.equal(ONE_TLOS.add(41000 * GAS_PRICE));
        });
    });
    describe(":: Request", function () {
        it("Should be able to calculate the request price correctly" , async function () {
            await expect(bridge.calculateRequestPrice(41000)).to.not.be.reverted;
            expect(await bridge.calculateRequestPrice(41000)).to.be.equal(ONE_TLOS.add(41000 * GAS_PRICE));
        });
        it("Should be able to create a request with correct parameters" , async function () {
            let cost = await bridge.calculateRequestPrice(41000);
            await expect(consumer.makeRequest("120000", "41000", 1, {"value": cost})).to.not.be.reverted;
        });
        it("Should emit a Requested event on request creation", async function () {
            let cost = await bridge.calculateRequestPrice(41000);
            await expect(consumer.makeRequest("120000", "41000", 1, {"value": cost})).to.emit(bridge, "Requested").withArgs(consumer.address, 0, 1);
        });
        it("Should revert request creation if the value passed is incorrect" , async function () {
            let cost = await bridge.calculateRequestPrice(20000);
            await expect(consumer.makeRequest("120000", 20000, 1, {"value": cost.div(2)})).to.be.reverted;
        });
        it("Should be able to delete a request by id" , async function () {
            let cost = await bridge.calculateRequestPrice(20000);
            await expect( consumer.makeRequest("120000", 20000, 1, {"value": cost })).to.not.be.reverted;
            await expect(bridge.deleteRequest(0)).to.not.be.reverted;
        });
        it("Should be able to delete a request by requestor and call id" , async function () {
            let cost = await bridge.calculateRequestPrice(20000);
            await expect(consumer.makeRequest("120000", 20000, 1, {"value": cost })).to.not.be.reverted;
            await expect(bridge.deleteRequestorRequest(consumer.address, 0)).to.not.be.reverted;
        });
        it("Should not be possible to have more than " + MAX_REQUESTS + " requests" , async function () {
            let cost = await bridge.calculateRequestPrice(20000);
            for(var i = 0; i < MAX_REQUESTS; i++){
                await expect(consumer.makeRequest("120000", 20000, 1, {"value": cost })).to.not.be.reverted;
            }
            await expect(consumer.makeRequest("120000", 20000, 1, {"value": cost })).to.be.reverted;
        });
    });
    describe(":: Response", function () {
        it("Shouldn't be able to reply from another address than the Antelope Oracle Bridge EVM address" , async function () {
            let cost = await bridge.calculateRequestPrice(20000);
            await expect(consumer.makeRequest("120000", 20000, 1, {"value": cost })).to.not.be.reverted;
            await expect(bridge.connect(user).reply(0, [116])).to.be.reverted;
        });
        it("Shouldn't be able to reply with an invalid callId parameter" , async function () {
            let cost = await bridge.calculateRequestPrice(50000);
            await expect(consumer.makeRequest("120000", 50000, 2, {"value": cost })).to.not.be.reverted;
            await expect(bridge.connect(oracle).reply(110, [116, 155])).to.be.reverted;
        });
        it("Should be able to reply to a Request" , async function () {
            let cost = await bridge.calculateRequestPrice(50000);
            await expect(consumer.makeRequest("120000", 50000, 2, {"value": cost })).to.not.be.reverted;
            await expect(bridge.connect(oracle).reply(0, [116, 155])).to.not.be.reverted;
        });
        it("Should be able to reply to a Request even with an empty numbers array" , async function () {
            let cost = await bridge.calculateRequestPrice(50000);
            await expect(consumer.makeRequest("120000", 50000, 2, {"value": cost })).to.not.be.reverted;
            await expect(bridge.connect(oracle).reply(0, [])).to.not.be.reverted;
        });
        it("Should emit a Replied event on valid reply", async function () {
            let cost = await bridge.calculateRequestPrice(50000);
            await expect(consumer.makeRequest("120000", 50000, 2, {"value": cost })).to.not.be.reverted;
            await expect(bridge.connect(oracle).reply(0, [116, 155])).to.emit(bridge, "Replied").withArgs(consumer.address, 0, [116, 155]);
        });
    });
});
